#pragma once

#include <cmath>

#include "MathUtils.hpp"

namespace granular {

inline float envelopeRectish(float phase01) {
    const float edge = 0.005f;
    if (phase01 < edge) {
        return phase01 / edge;
    }
    if (phase01 > 1.0f - edge) {
        return (1.0f - phase01) / edge;
    }
    return 1.0f;
}

inline float envelopeTriangle(float phase01) {
    return 1.0f - std::fabs(2.0f * phase01 - 1.0f);
}

inline float envelopeHann(float phase01) {
    return 0.5f - 0.5f * std::cos(2.0f * 3.14159265359f * phase01);
}

inline float envelopeGaussian(float phase01) {
    const float x = (phase01 - 0.5f) * 4.2f;
    return std::exp(-0.5f * x * x);
}

inline float envelopeSmear(float phase01) {
    // Reverse-swell style: slower rise + smooth late fade.
    const float rise = std::pow(phase01, 1.9f);
    const float body = 1.0f - 0.25f * std::pow(phase01, 0.65f);
    const float tail = std::pow(1.0f - phase01, 0.6f);
    return rise * body * tail;
}

inline float morph2(float a, float b, float t) {
    return a + (b - a) * clamp01(t);
}

inline float grainEnvelope(float phase01, float texture) {
    const float p = clamp01(phase01);
    const float t = clamp01(texture);

    const float rectish = envelopeRectish(p);
    const float tri = envelopeTriangle(p);
    const float hann = envelopeHann(p);
    const float gauss = envelopeGaussian(p);
    const float smear = envelopeSmear(p);

    float out = 0.0f;
    if (t < 0.2f) {
        out = morph2(rectish, tri, t / 0.2f);
    }
    else if (t < 0.45f) {
        out = morph2(tri, hann, (t - 0.2f) / 0.25f);
    }
    else if (t < 0.75f) {
        out = morph2(hann, gauss, (t - 0.45f) / 0.3f);
    }
    else {
        out = morph2(gauss, smear, (t - 0.75f) / 0.25f);
    }

    // Extra contrast: left side harder, right side softer.
    const float shapeExp = mapLinear(t, 0.72f, 1.45f);
    out = std::pow(clamp01(out), shapeExp);

    return clamp01(out);
}

} // namespace granular
