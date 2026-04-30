#pragma once

#include <cmath>

#include "MathUtils.hpp"

namespace granular {

inline float envelopeRectish(float phase01) {
    (void) phase01;
    // Hard rectangular window: absolutely no fade in/out.
    return 1.0f;
}

inline float envelopeTriangle(float phase01) {
    return 1.0f - std::fabs(2.0f * phase01 - 1.0f);
}

inline float envelopeHann(float phase01) {
    return 0.5f - 0.5f * std::cos(2.0f * 3.14159265359f * phase01);
}

inline float envelopeGaussian(float phase01) {
    const float x = (phase01 - 0.5f) * 3.4f;
    return std::exp(-0.5f * x * x);
}

inline float envelopeSmear(float phase01) {
    // Smear / reverse-swell style.
    const float rise = std::pow(phase01, 2.25f);
    const float body = 1.0f - 0.20f * std::pow(phase01, 0.55f);
    const float tail = std::pow(1.0f - phase01, 0.50f);
    return rise * body * tail;
}

inline float morph2(float a, float b, float t) {
    return a + (b - a) * clamp01(t);
}

inline float grainEnvelope(float phase01, float texture) {
    const float p = clamp01(phase01);
    const float t = clamp01(texture);

    const float rectish = envelopeRectish(p);
    const float hann = envelopeHann(p);
    const float gauss = envelopeGaussian(p);
    const float smear = envelopeSmear(p);

    float out = 0.0f;
    if (t < 0.5f) {
        // 0.0 -> 0.5 : no window -> rounded Hann.
        out = morph2(rectish, hann, t / 0.5f);
    }
    else if (t < 0.8f) {
        // 0.5 -> 0.8 : Hann -> Gaussian.
        out = morph2(hann, gauss, (t - 0.5f) / 0.3f);
    }
    else {
        // 0.8 -> 1.0 : Gaussian -> Smear.
        out = morph2(gauss, smear, (t - 0.8f) / 0.2f);
    }

    return clamp01(out);
}

} // namespace granular
