#pragma once

#include <array>

#include "FeedbackProcessor.hpp"
#include "Grain.hpp"
#include "GrainRenderer.hpp"
#include "GrainScheduler.hpp"
#include "GranularWorkstationParams.hpp"
#include "ReverbScWrapper.hpp"
#include "StereoRingBuffer.hpp"

namespace granular {

class GranularWorkstationEngine {
public:
    void setSampleRate(float sr);
    void reset();

    void process(float inL, float inR, float& outL, float& outR, const GranularWorkstationParams& p);

private:
    GranularWorkstationParams adaptForMode(const GranularWorkstationParams& p) const;

    float sampleRate_ = kDefaultSampleRate;
    float feedbackSendL_ = 0.0f;
    float feedbackSendR_ = 0.0f;

    StereoRingBuffer buffer_;
    GrainScheduler scheduler_;
    GrainRenderer renderer_;
    FeedbackProcessor feedback_;
    ReverbScWrapper reverb_;
    std::array<Grain, kMaxGrains> grains_{};
};

} // namespace granular
