#include <array>
#include "./permutation_parity.h"

static_assert(cobb::math::permutation_parity<std::array<int, 3>>({0, 1, 2}) == +1);
static_assert(cobb::math::permutation_parity<std::array<int, 3>>({0, 2, 1}) == -1);
static_assert(cobb::math::permutation_parity<std::array<int, 3>>({2, 0, 1}) == +1);