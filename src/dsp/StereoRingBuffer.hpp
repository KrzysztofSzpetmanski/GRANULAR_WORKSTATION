#pragma once

#include <array>

#include "GranularWorkstationParams.hpp"
#include "Interpolator.hpp"
#include "MathUtils.hpp"

namespace granular {

class StereoRingBuffer {
public:
    StereoRingBuffer() {
        clear();
    }

    void setSampleRate(float sampleRate) {
        sampleRate_ = (sampleRate > 1.0f) ? sampleRate : kDefaultSampleRate;
        int targetLength = static_cast<int>(sampleRate_ * kBufferSeconds);
        if (targetLength < 64) {
            targetLength = 64;
        }
        if (targetLength > kRingBufferSamples) {
            targetLength = kRingBufferSamples;
        }
        length_ = targetLength;
        writeIndex_ = 0;
        clear();
    }

    void clear() {
        left_.fill(0.0f);
        right_.fill(0.0f);
    }

    void write(float l, float r) {
        left_[writeIndex_] = sanitize(l);
        right_[writeIndex_] = sanitize(r);
        writeIndex_++;
        if (writeIndex_ >= length_) {
            writeIndex_ = 0;
        }
    }

    float readL(float index, InterpolationMode mode) const {
        return Interpolator::sample(left_.data(), length_, index, mode);
    }

    float readR(float index, InterpolationMode mode) const {
        return Interpolator::sample(right_.data(), length_, index, mode);
    }

    int wrapIndexInt(int index) const {
        return Interpolator::wrapIndex(index, length_);
    }

    float wrapIndexFloat(float index) const {
        const float len = static_cast<float>(length_);
        while (index < 0.0f) {
            index += len;
        }
        while (index >= len) {
            index -= len;
        }
        return index;
    }

    int getWriteIndex() const {
        return writeIndex_;
    }

    int getLength() const {
        return length_;
    }

    float getLengthMs() const {
        return static_cast<float>(length_) * 1000.0f / sampleRate_;
    }

private:
    std::array<float, kRingBufferSamples> left_{};
    std::array<float, kRingBufferSamples> right_{};

    int writeIndex_ = 0;
    int length_ = kRingBufferSamples;
    float sampleRate_ = kDefaultSampleRate;
};

} // namespace granular
