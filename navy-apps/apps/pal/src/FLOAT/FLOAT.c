#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>


FLOAT F_mul_F(FLOAT a, FLOAT b) {
  return ((int64_t)a * (int64_t)b) >> 16;
}


FLOAT F_div_F(FLOAT a, FLOAT b) {
  assert(b!=0);
  return ((int64_t)a<<16/b);
  // make sure dividor can not be 0

}


FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

    union {
    float f;
    uint32_t u;
  } v = { .f = a };

  uint32_t sign = v.u >> 31;
  int32_t exp = ((v.u >> 23) & 0xFF) - 127;
  uint32_t frac = v.u & 0x7FFFFF;  // 23 bits
  frac |= 1 << 23;  // 添加隐含的前导1

  // 调整为定点格式：我们希望保留16位小数 => 需要shift
  int shift = exp - 23 + 16;
  int32_t result;

  if (shift >= 0) {
    result = (int32_t)(frac << shift);
  } else {
    result = (int32_t)(frac >> -shift);
  }

  if (sign)
    result = -result;

  return result;
}



FLOAT Fabs(FLOAT a) {
  if ((a & 0x80000000) == 0) return a;
  else return (-a);
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);
 
  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
