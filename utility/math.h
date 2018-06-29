#ifndef FOOLGO_MATH_RAND_H_
#define FOOLGO_MATH_RAND_H_

#include <chrono>
#include <cstdint>
#include <vector>
#include "../board/def.h"

namespace foolgo {
namespace math {

uint32_t Rand(uint32_t max, uint32_t seed);

inline uint32_t GetTimeSeed() {
    static unsigned time_seed =
        std::chrono::system_clock::now().time_since_epoch().count();
    return time_seed;
}

template <int LEN>
std::vector<board::PositionIndex> GetOnePositionIndexes(const std::bitset<LEN> &b) {
    board::PositionIndex c = b.count();
    std::vector<board::PositionIndex> v(c);
    board::PositionIndex vi = 0;

    for (int i = 0; i < b.size(); ++i) {
        if (b[i]) {
            v[vi++] = i;
        }
    }

    return v;
}

} // namespace math
} // namespace foolgo

#endif