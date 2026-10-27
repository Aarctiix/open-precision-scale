import { test } from 'node:test';
import assert from 'node:assert/strict';
import { bmr, tdee, macroSplit, portion } from '../src/index.js';

const close = (a, b, eps = 1e-9) => assert.ok(Math.abs(a - b) < eps, `${a} != ${b}`);

test('bmr matches Mifflin-St Jeor', () => {
  close(bmr({ sex: 'male', weightKg: 80, heightCm: 180, age: 30 }), 1780);
  close(bmr({ sex: 'female', weightKg: 60, heightCm: 165, age: 25 }), 1345.25);
});

test('bmr rejects bad input', () => {
  assert.throws(() => bmr({ sex: 'male', weightKg: 0, heightCm: 180, age: 30 }), RangeError);
  assert.throws(() => bmr({ sex: 'x', weightKg: 80, heightCm: 180, age: 30 }), RangeError);
});

test('tdee applies activity factor', () => {
  const p = { sex: 'male', weightKg: 80, heightCm: 180, age: 30 };
  close(tdee(p, 'moderate'), 1780 * 1.55);
  close(tdee(p, 1.3), 1780 * 1.3);
  assert.throws(() => tdee(p, 'couch'), RangeError);
});

test('macroSplit converts calories to grams', () => {
  const m = macroSplit(2000);
  close(m.protein, 150);
  close(m.carbs, 200);
  close(m.fat, 600 / 9);
  assert.throws(() => macroSplit(2000, { protein: 0.5, carbs: 0.5, fat: 0.5 }));
});

test('portion scales per-100g values', () => {
  // 37.5 g of oats
  const p = portion(37.5, { kcal: 372, protein: 13.5, carbs: 58.7, fat: 7 });
  close(p.kcal, 139.5);
  close(p.protein, 5.0625);
  close(portion(0, { kcal: 100 }).kcal, 0);
  assert.throws(() => portion(-1, { kcal: 100 }));
});
