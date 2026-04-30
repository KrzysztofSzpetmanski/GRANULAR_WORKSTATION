#pragma once

#include <array>

#include "MathUtils.hpp"

namespace granular {

template <int N>
class SimpleDelayLine {
public:
    void clear() {
        buffer_.fill(0.0f);
        index_ = 0;
    }

    float read() const {
        return buffer_[index_];
    }

    void write(float x) {
        buffer_[index_] = x;
        index_++;
        if (index_ >= length_) {
            index_ = 0;
        }
    }

    void setLength(int length) {
        if (length < 1) {
            length = 1;
        }
        if (length > N) {
            length = N;
        }
        length_ = length;
        if (index_ >= length_) {
            index_ = 0;
        }
    }

private:
    std::array<float, N> buffer_{};
    int index_ = 0;
    int length_ = N;
};

class ReverbScWrapper {
public:
    void init(float sampleRate) {
        sampleRate_ = (sampleRate > 1.0f) ? sampleRate : 48000.0f;
        const float scale = sampleRate_ / 48000.0f;

        combL1_.setLength(static_cast<int>(1116 * scale));
        combL2_.setLength(static_cast<int>(1188 * scale));
        combL3_.setLength(static_cast<int>(1277 * scale));
        combL4_.setLength(static_cast<int>(1356 * scale));

        combR1_.setLength(static_cast<int>(1139 * scale));
        combR2_.setLength(static_cast<int>(1211 * scale));
        combR3_.setLength(static_cast<int>(1300 * scale));
        combR4_.setLength(static_cast<int>(1379 * scale));

        apL1_.setLength(static_cast<int>(225 * scale));
        apL2_.setLength(static_cast<int>(556 * scale));
        apR1_.setLength(static_cast<int>(248 * scale));
        apR2_.setLength(static_cast<int>(579 * scale));

        reset();
    }

    void reset() {
        combL1_.clear();
        combL2_.clear();
        combL3_.clear();
        combL4_.clear();
        combR1_.clear();
        combR2_.clear();
        combR3_.clear();
        combR4_.clear();
        apL1_.clear();
        apL2_.clear();
        apR1_.clear();
        apR2_.clear();

        filterStoreL_.fill(0.0f);
        filterStoreR_.fill(0.0f);
    }

    void setParams(float reverbAmount, float damp) {
        wet_ = clamp01(reverbAmount);
        room_ = mapLinear(reverbAmount, 0.45f, 0.86f);
        damp_ = mapLinear(1.0f - damp, 0.05f, 0.75f);
    }

    void process(float inL, float inR, float& outL, float& outR) {
        const float xL = sanitize(inL);
        const float xR = sanitize(inR);
        const float input = 0.015f * (xL + xR);

        float sumL = 0.0f;
        float sumR = 0.0f;

        sumL += combProcess(combL1_, filterStoreL_[0], input);
        sumL += combProcess(combL2_, filterStoreL_[1], input);
        sumL += combProcess(combL3_, filterStoreL_[2], input);
        sumL += combProcess(combL4_, filterStoreL_[3], input);

        sumR += combProcess(combR1_, filterStoreR_[0], input);
        sumR += combProcess(combR2_, filterStoreR_[1], input);
        sumR += combProcess(combR3_, filterStoreR_[2], input);
        sumR += combProcess(combR4_, filterStoreR_[3], input);

        float yL = allpassProcess(apL1_, sumL, 0.5f);
        yL = allpassProcess(apL2_, yL, 0.5f);

        float yR = allpassProcess(apR1_, sumR, 0.5f);
        yR = allpassProcess(apR2_, yR, 0.5f);

        outL = yL * wet_;
        outR = yR * wet_;
    }

private:
    float combProcess(SimpleDelayLine<4096>& delay, float& filterStore, float input) {
        const float output = delay.read();
        filterStore = output * (1.0f - damp_) + filterStore * damp_;
        delay.write(input + filterStore * room_);
        return output;
    }

    float allpassProcess(SimpleDelayLine<1024>& delay, float input, float feedback) {
        const float buffered = delay.read();
        const float out = -input + buffered;
        delay.write(input + buffered * feedback);
        return out;
    }

    float sampleRate_ = 48000.0f;
    float wet_ = 0.5f;
    float room_ = 0.65f;
    float damp_ = 0.3f;

    SimpleDelayLine<4096> combL1_;
    SimpleDelayLine<4096> combL2_;
    SimpleDelayLine<4096> combL3_;
    SimpleDelayLine<4096> combL4_;
    SimpleDelayLine<4096> combR1_;
    SimpleDelayLine<4096> combR2_;
    SimpleDelayLine<4096> combR3_;
    SimpleDelayLine<4096> combR4_;

    SimpleDelayLine<1024> apL1_;
    SimpleDelayLine<1024> apL2_;
    SimpleDelayLine<1024> apR1_;
    SimpleDelayLine<1024> apR2_;

    std::array<float, 4> filterStoreL_{};
    std::array<float, 4> filterStoreR_{};
};

} // namespace granular
