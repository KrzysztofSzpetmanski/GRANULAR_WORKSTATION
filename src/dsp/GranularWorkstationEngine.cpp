#include "GranularWorkstationEngine.hpp"

#include <algorithm>

#include "MathUtils.hpp"

namespace granular {

void GranularWorkstationEngine::setSampleRate(float sr) {
    sampleRate_ = (sr > 1.0f) ? sr : kDefaultSampleRate;
    buffer_.setSampleRate(sampleRate_);
    scheduler_.setSampleRate(sampleRate_);
    renderer_.setSampleRate(sampleRate_);
    feedback_.setSampleRate(sampleRate_);
    reverb_.init(sampleRate_);
}

void GranularWorkstationEngine::reset() {
    buffer_.clear();
    scheduler_.reset();
    renderer_.reset();
    feedback_.reset();
    reverb_.reset();
    feedbackSendL_ = 0.0f;
    feedbackSendR_ = 0.0f;

    for (Grain& g : grains_) {
        g = Grain{};
    }
}

GranularWorkstationParams GranularWorkstationEngine::adaptForMode(const GranularWorkstationParams& p) const {
    GranularWorkstationParams m = p;

    switch (p.mode) {
        case GranMode::GhostDelay:
            m.overlap = clamp01(m.overlap * 0.65f);
            m.reverb = clamp01(m.reverb * 0.65f);
            break;
        case GranMode::FreezeSpace:
            m.overlap = clamp01(m.overlap * 1.2f);
            m.reverb = clamp01(m.reverb * 1.15f);
            break;
        case GranMode::TimeStretch:
            m.density = clamp01(m.density * 0.8f);
            m.overlap = clamp01(m.overlap * 1.1f);
            break;
        case GranMode::PitchShift:
            m.density = clamp01(m.density * 1.05f);
            break;
        case GranMode::MicroLoop:
            m.overlap = clamp01(m.overlap * 1.25f);
            m.positionSpread = clamp01(m.positionSpread * 0.6f);
            break;
        case GranMode::Pulsar:
            m.density = clamp01(m.density * 1.2f);
            m.overlap = clamp01(m.overlap * 0.9f);
            break;
        case GranMode::CloudRev:
        default:
            break;
    }

    return m;
}

void GranularWorkstationEngine::process(float inL, float inR, float& outL, float& outR, const GranularWorkstationParams& p) {
    const GranularWorkstationParams m = adaptForMode(p);

    float xL = sanitize(inL);
    float xR = sanitize(inR);

    xL = softClip(0.98f * xL);
    xR = softClip(0.98f * xR);

    const float writeL = m.freeze ? feedbackSendL_ : (xL + feedbackSendL_);
    const float writeR = m.freeze ? feedbackSendR_ : (xR + feedbackSendR_);
    buffer_.write(softClip(writeL), softClip(writeR));

    const int spawns = scheduler_.process(m);
    for (int i = 0; i < spawns; ++i) {
        renderer_.spawnGrain(grains_, buffer_, m);
    }

    float grainWetL = 0.0f;
    float grainWetR = 0.0f;
    renderer_.renderGrains(grains_, buffer_, m.space, grainWetL, grainWetR);

    reverb_.setParams(m.reverb, m.damp);
    float reverbL = 0.0f;
    float reverbR = 0.0f;
    reverb_.process(grainWetL, grainWetR, reverbL, reverbR);

    feedback_.process(reverbL, reverbR, m.feedback, m.damp, feedbackSendL_, feedbackSendR_);

    const float mix = clamp01(m.mix);
    const float wetL = sanitize(reverbL + grainWetL * (1.0f - m.reverb));
    const float wetR = sanitize(reverbR + grainWetR * (1.0f - m.reverb));

    outL = softClip(xL * (1.0f - mix) + wetL * mix);
    outR = softClip(xR * (1.0f - mix) + wetR * mix);

    outL = sanitize(outL);
    outR = sanitize(outR);
}

} // namespace granular
