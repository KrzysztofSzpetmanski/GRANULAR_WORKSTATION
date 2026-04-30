#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "Grain.hpp"
#include "GrainEnvelope.hpp"
#include "GranularWorkstationParams.hpp"
#include "Interpolator.hpp"
#include "MathUtils.hpp"
#include "Random.hpp"
#include "StereoRingBuffer.hpp"

namespace granular {

class GrainRenderer {
public:
    void setSampleRate(float sr) {
        sampleRate_ = (sr > 1.0f) ? sr : kDefaultSampleRate;
    }

    void reset() {
        rng_.reseed(0x9E3779B9u);
    }

    void spawnGrain(std::array<Grain, kMaxGrains>& grains, const StereoRingBuffer& buffer, const GranularWorkstationParams& p) {
        const int activeCount = countActiveGrains(grains);
        const float activeLimitF = mapLinear(p.overlap, 8.0f, static_cast<float>(kMaxGrains));
        const int activeLimit = std::max(4, static_cast<int>(activeLimitF));
        if (activeCount >= activeLimit) {
            return;
        }

        int index = findFreeGrain(grains);
        if (index < 0) {
            return;
        }

        Grain& g = grains[index];
        g.active = true;
        g.ageSamples = 0;
        g.texture = clamp01(p.texture);
        g.reverse = (rng_.nextFloat01() < clamp01(p.reverseProb));

        float grainMs = mapExp(p.size, 5.0f, 1000.0f);
        if (p.mode == GranMode::MicroLoop) {
            grainMs = mapExp(p.size, 4.0f, 140.0f);
        }

        const float overlapScale = mapLinear(p.overlap, 0.7f, 2.5f);
        const float modeScale = modeDurationScale(p.mode);
        const float effectiveMs = grainMs * overlapScale * modeScale;
        g.durationSamples = std::max(8, static_cast<int>(effectiveMs * sampleRate_ * 0.001f));

        const float pitchSemi = p.pitch * 24.0f;
        const float pitchRatio = semiToRatio(pitchSemi);
        g.step = g.reverse ? -pitchRatio : pitchRatio;

        const float maxTailMs = mapExp(p.positionSpread, 20.0f, buffer.getLengthMs() * 0.95f);
        const float ageMs = selectAgeMs(p, maxTailMs);

        const float ageJitterMs = rng_.nextSigned() * p.space * 0.08f * maxTailMs;
        const float ageL = std::max(1.0f, ageMs + ageJitterMs);
        const float ageR = std::max(1.0f, ageMs - ageJitterMs);

        const float readOffsetLSamples = ageL * sampleRate_ * 0.001f;
        const float readOffsetRSamples = ageR * sampleRate_ * 0.001f;

        const float writeIndex = static_cast<float>(buffer.getWriteIndex());
        g.posL = buffer.wrapIndexFloat(writeIndex - readOffsetLSamples);
        g.posR = buffer.wrapIndexFloat(writeIndex - readOffsetRSamples);

        const float panWidth = modePanWidth(p.mode) * clamp01(p.space);
        g.pan = rng_.nextSigned() * panWidth;

        const float densityHz = mapExp(p.density, 0.5f, 250.0f);
        float estimatedActive = std::max(1.0f, densityHz * (effectiveMs * 0.001f));
        estimatedActive = std::min(estimatedActive, activeLimitF);
        const float gainComp = 1.2f / std::sqrt(estimatedActive);
        g.amp = std::min(1.1f, gainComp);
    }

    void renderGrains(std::array<Grain, kMaxGrains>& grains, const StereoRingBuffer& buffer, float space, float& wetL, float& wetR) {
        wetL = 0.0f;
        wetR = 0.0f;

        for (Grain& g : grains) {
            if (!g.active) {
                continue;
            }

            if (g.ageSamples >= g.durationSamples) {
                g.active = false;
                continue;
            }

            const float phase = static_cast<float>(g.ageSamples) / static_cast<float>(g.durationSamples);
            const float env = grainEnvelope(phase, g.texture);

            const InterpolationMode interp = (g.texture < 0.2f) ? InterpolationMode::Linear : InterpolationMode::CubicHermite;
            const float smear = clamp01((g.texture - 0.72f) * (1.0f / 0.28f));
            const float smearOffset = smear * 1.25f * std::sin(phase * (18.0f + smear * 27.0f));

            float sL = buffer.readL(g.posL, interp);
            float sR = buffer.readR(g.posR, interp);
            if (smear > 0.001f) {
                const float blurL = buffer.readL(g.posL - smearOffset, interp);
                const float blurR = buffer.readR(g.posR + smearOffset, interp);
                const float blend = smear * 0.55f;
                sL = sL + (blurL - sL) * blend;
                sR = sR + (blurR - sR) * blend;
            }

            float mono = 0.5f * (sL + sR);
            mono *= g.amp * env;

            const float angle = (g.pan + 1.0f) * 0.25f * 3.14159265359f;
            const float panL = std::cos(angle);
            const float panR = std::sin(angle);

            wetL += mono * panL;
            wetR += mono * panR;

            g.posL = buffer.wrapIndexFloat(g.posL + g.step);
            g.posR = buffer.wrapIndexFloat(g.posR + g.step);
            g.ageSamples++;
        }

        applyStereoSpace(wetL, wetR, clamp01(space));

        wetL = sanitize(wetL);
        wetR = sanitize(wetR);
    }

private:
    int findFreeGrain(const std::array<Grain, kMaxGrains>& grains) const {
        for (int i = 0; i < kMaxGrains; ++i) {
            if (!grains[static_cast<size_t>(i)].active) {
                return i;
            }
        }
        return -1;
    }

    int countActiveGrains(const std::array<Grain, kMaxGrains>& grains) const {
        int count = 0;
        for (int i = 0; i < kMaxGrains; ++i) {
            if (grains[static_cast<size_t>(i)].active) {
                count++;
            }
        }
        return count;
    }

    float selectAgeMs(const GranularWorkstationParams& p, float maxTailMs) {
        float ageMs = rng_.nextRange(5.0f, std::max(6.0f, maxTailMs));
        if (p.mode == GranMode::TimeStretch) {
            const float center = mapLinear(p.positionSpread, 10.0f, maxTailMs);
            ageMs = center + rng_.nextSigned() * 0.15f * maxTailMs;
        }
        return std::max(5.0f, ageMs);
    }

    float modeDurationScale(GranMode mode) const {
        switch (mode) {
            case GranMode::GhostDelay:
                return 0.85f;
            case GranMode::FreezeSpace:
                return 1.05f;
            case GranMode::TimeStretch:
                return 1.25f;
            case GranMode::PitchShift:
                return 1.0f;
            case GranMode::MicroLoop:
                return 0.45f;
            case GranMode::Pulsar:
                return 0.75f;
            case GranMode::CloudRev:
            default:
                return 1.0f;
        }
    }

    float modePanWidth(GranMode mode) const {
        switch (mode) {
            case GranMode::GhostDelay:
                return 0.6f;
            case GranMode::FreezeSpace:
                return 0.9f;
            case GranMode::TimeStretch:
                return 0.7f;
            case GranMode::PitchShift:
                return 0.75f;
            case GranMode::MicroLoop:
                return 0.55f;
            case GranMode::Pulsar:
                return 1.0f;
            case GranMode::CloudRev:
            default:
                return 0.8f;
        }
    }

    void applyStereoSpace(float& l, float& r, float space) const {
        const float mid = 0.5f * (l + r);
        const float side = 0.5f * (l - r);
        const float width = mapLinear(space, 0.0f, 1.75f);
        l = mid + side * width;
        r = mid - side * width;
    }

    float sampleRate_ = kDefaultSampleRate;
    Random rng_{0xCAFEBABEu};
};

} // namespace granular
