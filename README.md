# open-precision-scale

Open firmware for a high-precision kitchen scale (ESP32 + HX711), plus a tiny
nutrition library that turns a weighed portion into calories and macros.

The idea: put food on the scale, get a stable reading to a tenth of a gram,
and send it over BLE to a nutrition app (I use it with my own app, FitTrack)
that does the math.

```
firmware/    ESP32 + HX711 firmware (PlatformIO, Arduino framework)
nutrition/   BMR / TDEE / macros / portion helpers, plain JS, zero deps
```

## Firmware

Features:

- tare, and multi-point calibration (up to 5 points, piecewise linear) saved in NVS
- moving average with median-based outlier rejection
- stability flag (window spread under 0.2 g)
- grams or ounces
- JSON output on serial and on a BLE characteristic (read + notify)

### Wiring

```
 Load cell        HX711            ESP32
 ---------        -----            -----
 red    (E+) ---- E+
 black  (E-) ---- E-
 white  (A-) ---- A-
 green  (A+) ---- A+
                  VCC  ----------- 3V3
                  GND  ----------- GND
                  DT   ----------- GPIO16
                  SCK  ----------- GPIO4
```

Wire colours vary between load cells; if the reading goes negative under load, swap A+ and A-.

### Build and flash

```
cd firmware
pio run -t upload
pio device monitor
```

Serial commands (115200 baud): `t` tare, `c 100` add a calibration point with
100 g on the pan, `x` clear calibration, `u` toggle g/oz.

Calibrate with an empty pan: `t`, then put a known mass and send `c <grams>`.
Add more points across the range (e.g. 100 g, 500 g, 2000 g) for better linearity.

Output, both on serial and BLE:

```json
{"w":137.42,"u":"g","s":true}
```

BLE service `6e0a0001-5c1e-4b8a-9d3c-0a1b2c3d4e5f`, weight characteristic
`6e0a0002-5c1e-4b8a-9d3c-0a1b2c3d4e5f`.

## Nutrition library

```js
import { tdee, macroSplit, portion } from './nutrition/src/index.js';

const kcal = tdee({ sex: 'male', weightKg: 78, heightCm: 180, age: 22 }, 'moderate');
macroSplit(kcal);                                   // grams of protein/carbs/fat
portion(137.4, { kcal: 372, protein: 13.5, carbs: 58.7, fat: 7 });
```

Run the tests with `cd nutrition && npm test` (Node 18+).

## Status and roadmap

This is a solo project: I write the firmware and the software on my own as an
independent developer. The longer-term plan is a small hardware brand of very
precise kitchen scales built on this code, but that part isn't launched yet,
mostly because I don't have the funding for manufacturing. For now the code is
open and works on off-the-shelf parts.

- [x] HX711 reading, tare, multi-point calibration
- [x] filtering and stability detection
- [x] BLE weight characteristic
- [ ] temperature drift compensation
- [ ] auto-zero tracking
- [ ] OLED display and buttons
- [ ] BLE commands (tare / unit) from the app
- [ ] food database lookup in the nutrition lib
- [ ] PCB and enclosure files

## En français

Firmware open source pour une balance de cuisine de précision (ESP32 + HX711)
et petite librairie nutrition (métabolisme de base, TDEE, macros, valeurs d'une
portion pesée). Projet développé en solo ; la marque de balances est prévue
mais pas encore lancée, faute de financement pour la fabrication.

## License

MIT, see [LICENSE](LICENSE).
