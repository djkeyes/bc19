

#ifndef CPP_STARTER_FAST_RAND_H
#define CPP_STARTER_FAST_RAND_H

#include <cstdint>

namespace fast_rand {

static uint32_t g_seed = 2019;
static constexpr uint32_t rand_max = 0x7FFF;

inline void srand(const unsigned int seed) {
  g_seed = seed;
}

// Output value in range [0, 32767] = [0, 2^15-1]
inline uint32_t rand() {
  g_seed = (214013 * g_seed + 2531011);
  return (g_seed >> 16) & rand_max;
}

// Outputs a value in the range [0, max_val).
// This is approximately uniform if uint32_t << fast_rand::rand_max.
inline uint32_t small_uniform_int_rand(const uint32_t max_val) {
  return rand() % max_val;
}

}

#endif //CPP_STARTER_FAST_RAND_H
