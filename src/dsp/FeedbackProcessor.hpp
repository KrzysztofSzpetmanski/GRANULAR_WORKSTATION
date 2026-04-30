#pragma once

#include "MathUtils.hpp"
#include "OnePoleFilters.hpp"
#include "SoftClip.hpp"

namespace granular {

class FeedbackProcessor {
public:
    void setSampleRate(float sr) {
        sampleRate_ = (sr > 1.0f) ? sr : 48000.0f;
        lpL_.setSampleRate(sampleRate_);
        lpR_.setSampleRate(sampleRate_);
        hpL_.setSampleRate(sampleRate_);
        hpR_.setSampleRate(sampleRate_);
        hpL_.setCutoff(90.0f);
        hpR_.setCutoff(90.0f);
    }

    void reset() {
        lpL_.reset();
        lpR_.reset();
        hpL_.reset();
        hpR_.reset();
    }

    void process(float inL, float inR, float feedbackAmount, float damp, float& outL, float& outR) {
        const float safeFeedback = clamp01(feedbackAmount);
        const float lpCutoff = mapExp(damp, 800.0f, 16000.0f);

        lpL_.setCutoff(lpCutoff);
        lpR_.setCutoff(lpCutoff);

        float xL = sanitize(inL);
        float xR = sanitize(inR);

        xL = lpL_.process(xL);
        xR = lpR_.process(xR);

        xL = hpL_.process(xL);
        xR = hpR_.process(xR);

        const float conservativeFeedback = safeFeedback * 0.97f;
        xL *= conservativeFeedback;
        xR *= conservativeFeedback;

        outL = softClipSample(xL);
        outR = softClipSample(xR);
    }

private:
    float sampleRate_ = 48000.0f;
    OnePoleLowpass lpL_;
    OnePoleLowpass lpR_;
    OnePoleHighpass hpL_;
    OnePoleHighpass hpR_;
};

} // namespace granular
