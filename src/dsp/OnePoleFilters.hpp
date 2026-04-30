#pragma once

#include <cmath>

namespace granular {

class OnePoleLowpass {
public:
    void setSampleRate(float sr) {
        sampleRate_ = (sr > 1.0f) ? sr : 48000.0f;
        updateCoeff();
    }

    void setCutoff(float hz) {
        cutoffHz_ = (hz > 1.0f) ? hz : 1.0f;
        updateCoeff();
    }

    float process(float x) {
        z_ += a_ * (x - z_);
        return z_;
    }

    void reset(float x = 0.0f) {
        z_ = x;
    }

private:
    void updateCoeff() {
        const float x = -2.0f * 3.14159265359f * cutoffHz_ / sampleRate_;
        a_ = 1.0f - std::exp(x);
    }

    float sampleRate_ = 48000.0f;
    float cutoffHz_ = 4000.0f;
    float a_ = 0.2f;
    float z_ = 0.0f;
};

class OnePoleHighpass {
public:
    void setSampleRate(float sr) {
        sampleRate_ = (sr > 1.0f) ? sr : 48000.0f;
        updateCoeff();
    }

    void setCutoff(float hz) {
        cutoffHz_ = (hz > 1.0f) ? hz : 1.0f;
        updateCoeff();
    }

    float process(float x) {
        const float y = a_ * (zY_ + x - zX_);
        zX_ = x;
        zY_ = y;
        return y;
    }

    void reset() {
        zX_ = 0.0f;
        zY_ = 0.0f;
    }

private:
    void updateCoeff() {
        const float wc = 2.0f * 3.14159265359f * cutoffHz_;
        const float t = 1.0f / sampleRate_;
        a_ = 1.0f / (1.0f + wc * t);
    }

    float sampleRate_ = 48000.0f;
    float cutoffHz_ = 90.0f;
    float a_ = 0.99f;
    float zX_ = 0.0f;
    float zY_ = 0.0f;
};

} // namespace granular
