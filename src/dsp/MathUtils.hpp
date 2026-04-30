#pragma once

#include <cmath>

namespace granular {

inline float clamp01(float x) {
    if (x < 0.0f) {
        return 0.0f;
    }
    if (x > 1.0f) {
        return 1.0f;
    }
    return x;
}

inline float clampSigned(float x, float limit = 1.0f) {
    if (x < -limit) {
        return -limit;
    }
    if (x > limit) {
        return limit;
    }
    return x;
}

inline float mapLinear(float x, float a, float b) {
    return a + (b - a) * clamp01(x);
}

inline float mapExp(float x, float minVal, float maxVal) {
    const float t = clamp01(x);
    const float minSafe = (minVal <= 0.0f) ? 0.0001f : minVal;
    const float maxSafe = (maxVal <= minSafe) ? (minSafe + 0.0001f) : maxVal;
    return minSafe * std::pow(maxSafe / minSafe, t);
}

inline float semiToRatio(float semi) {
    return std::pow(2.0f, semi / 12.0f);
}

inline float softClip(float x) {
    return x / (1.0f + std::fabs(x));
}

inline float sanitize(float x) {
    if (!std::isfinite(x)) {
        return 0.0f;
    }
    return x;
}

} // namespace granular
