#pragma once

#include <cstdint>

namespace granular {

class Random {
public:
    explicit Random(uint32_t seed = 0x12345678u) : state_(seed ? seed : 0x12345678u) {
    }

    void reseed(uint32_t seed) {
        state_ = seed ? seed : 0x12345678u;
    }

    uint32_t nextUInt() {
        uint32_t x = state_;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state_ = x;
        return x;
    }

    float nextFloat01() {
        return static_cast<float>((nextUInt() >> 8) & 0x00FFFFFFu) * (1.0f / 16777216.0f);
    }

    float nextRange(float minVal, float maxVal) {
        return minVal + (maxVal - minVal) * nextFloat01();
    }

    float nextSigned() {
        return nextFloat01() * 2.0f - 1.0f;
    }

private:
    uint32_t state_;
};

} // namespace granular
