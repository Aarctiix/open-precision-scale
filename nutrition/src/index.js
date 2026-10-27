// Small nutrition helpers shared by the scale and the FitTrack app.
// Units: kg, cm, years, kcal, grams.

/** @typedef {'male'|'female'} Sex */

/** Activity multipliers commonly used with Mifflin-St Jeor. */
export const ACTIVITY = Object.freeze({
  sedentary: 1.2,
  light: 1.375,
  moderate: 1.55,
  active: 1.725,
  veryActive: 1.9,
});

/**
 * Basal metabolic rate, Mifflin-St Jeor (1990).
 * @param {{ sex: Sex, weightKg: number, heightCm: number, age: number }} p
 * @returns {number} kcal/day
 */
export function bmr({ sex, weightKg, heightCm, age }) {
  for (const [k, v] of Object.entries({ weightKg, heightCm, age })) {
    if (!(v > 0)) throw new RangeError(`${k} must be a positive number`);
  }
  const base = 10 * weightKg + 6.25 * heightCm - 5 * age;
  if (sex === 'male') return base + 5;
  if (sex === 'female') return base - 161;
  throw new RangeError(`unknown sex: ${sex}`);
}

/**
 * Total daily energy expenditure.
 * @param {Parameters<typeof bmr>[0]} p
 * @param {keyof typeof ACTIVITY | number} activity name or raw factor
 */
export function tdee(p, activity = 'sedentary') {
  const f = typeof activity === 'number' ? activity : ACTIVITY[activity];
  if (!f) throw new RangeError(`unknown activity level: ${activity}`);
  return bmr(p) * f;
}

/**
 * Split a calorie target into grams of protein / carbs / fat.
 * Ratios are fractions of calories and must add up to 1.
 * @param {number} kcal
 * @param {{ protein: number, carbs: number, fat: number }} [ratio]
 */
export function macroSplit(kcal, ratio = { protein: 0.3, carbs: 0.4, fat: 0.3 }) {
  const sum = ratio.protein + ratio.carbs + ratio.fat;
  if (Math.abs(sum - 1) > 1e-6) throw new RangeError('macro ratios must sum to 1');
  return {
    protein: (kcal * ratio.protein) / 4,
    carbs: (kcal * ratio.carbs) / 4,
    fat: (kcal * ratio.fat) / 9,
  };
}

/**
 * Nutrition of a weighed portion, from label values per 100 g.
 * @param {number} grams weight on the scale
 * @param {{ kcal: number, protein?: number, carbs?: number, fat?: number }} per100
 */
export function portion(grams, per100) {
  if (!(grams >= 0)) throw new RangeError('grams must be >= 0');
  const k = grams / 100;
  return {
    kcal: per100.kcal * k,
    protein: (per100.protein ?? 0) * k,
    carbs: (per100.carbs ?? 0) * k,
    fat: (per100.fat ?? 0) * k,
  };
}
