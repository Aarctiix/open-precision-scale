// open-precision-scale firmware
// ESP32 + HX711. Serial commands (115200 baud, one per line):
//   t           tare
//   c <grams>   add a calibration point with a known mass on the pan
//   x           clear calibration
//   u           toggle g / oz
#include <Arduino.h>
#include <HX711.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include "calibration.h"
#include "filter.h"

static const int PIN_DOUT = 16;
static const int PIN_SCK = 4;

static const float STABLE_SPREAD_G = 0.2f;  // window spread below this = stable
static const float GRAMS_PER_OZ = 28.349523f;

#define SERVICE_UUID "6e0a0001-5c1e-4b8a-9d3c-0a1b2c3d4e5f"
#define WEIGHT_UUID  "6e0a0002-5c1e-4b8a-9d3c-0a1b2c3d4e5f"

HX711 hx;
Calibration cal;
WeightFilter<10> filter(5.0f);
BLECharacteristic *weightChar = nullptr;

long tareOffset = 0;
bool useOz = false;
float lastRaw = 0;

void tare() {
  tareOffset = hx.read_average(20);
  filter.reset();
  Serial.println("# tared");
}

void handleSerial() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line == "t") {
    tare();
  } else if (line.startsWith("c ")) {
    float g = line.substring(2).toFloat();
    float raw = filter.mean();
    if (cal.add(raw, g)) {
      cal.save();
      Serial.printf("# cal point %u: raw=%.1f -> %.2f g\n", cal.size(), raw, g);
    } else {
      Serial.println("# cal point rejected");
    }
  } else if (line == "x") {
    cal.clear();
    cal.save();
    Serial.println("# calibration cleared");
  } else if (line == "u") {
    useOz = !useOz;
  }
}

void setupBle() {
  BLEDevice::init("OpenPrecisionScale");
  BLEServer *server = BLEDevice::createServer();
  BLEService *svc = server->createService(SERVICE_UUID);
  weightChar = svc->createCharacteristic(
      WEIGHT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  weightChar->addDescriptor(new BLE2902());
  svc->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
}

void setup() {
  Serial.begin(115200);
  hx.begin(PIN_DOUT, PIN_SCK);
  hx.set_gain(128);
  delay(500);  // let the load cell settle after power-up
  if (!cal.load()) Serial.println("# no calibration stored, showing raw counts");
  tare();
  setupBle();
}

void loop() {
  handleSerial();
  if (!hx.is_ready()) return;

  float raw = (float)(hx.read() - tareOffset);
  filter.push(raw);
  if (!filter.full()) return;

  float grams = cal.toGrams(filter.mean());
  float rawSpread = filter.spread();
  // convert the raw spread to grams using the local slope
  float gSpread = fabsf(cal.toGrams(filter.mean() + rawSpread) - grams);
  bool stable = gSpread < STABLE_SPREAD_G;

  float value = useOz ? grams / GRAMS_PER_OZ : grams;
  const char *unit = useOz ? "oz" : "g";

  char json[80];
  snprintf(json, sizeof(json), "{\"w\":%.2f,\"u\":\"%s\",\"s\":%s}",
           value, unit, stable ? "true" : "false");
  Serial.println(json);

  if (weightChar) {
    weightChar->setValue((uint8_t *)json, strlen(json));
    weightChar->notify();
  }
}
