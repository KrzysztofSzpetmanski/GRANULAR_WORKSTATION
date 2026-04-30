#pragma once

#include <cmath>

#include "GranularWorkstationParams.hpp"
#include "MathUtils.hpp"

namespace granular {

class GrainScheduler {
public:
    void setSampleRate(float sr) {
        sampleRate_ = (sr > 1.0f) ? sr : kDefaultSampleRate;
        accumulator_ = 0.0f;
    }

    void reset() {
        accumulator_ = 0.0f;
    }

    int process(const GranularWorkstationParams& p) {
        float densityHz = mapExp(p.density, 0.5f, 250.0f);

        switch (p.mode) {
            case GranMode::GhostDelay:
                densityHz *= 0.4f;
                break;
            case GranMode::FreezeSpace:
                densityHz *= 0.9f;
                break;
            case GranMode::TimeStretch:
                densityHz *= 0.8f;
                break;
            case GranMode::PitchShift:
                densityHz *= 1.05f;
                break;
            case GranMode::MicroLoop:
                densityHz *= 1.35f;
                break;
            case GranMode::Pulsar:
                densityHz *= 1.2f;
                break;
            case GranMode::CloudRev:
            default:
                break;
        }

        accumulator_ += densityHz / sampleRate_;

        int spawns = static_cast<int>(std::floor(accumulator_));
        if (spawns > 0) {
            accumulator_ -= static_cast<float>(spawns);
        }

        if (p.clockTick) {
            spawns += clockBurstForMode(p.mode);
        }

        if (spawns > 8) {
            spawns = 8;
        }
        if (spawns < 0) {
            spawns = 0;
        }

        return spawns;
    }

private:
    int clockBurstForMode(GranMode mode) const {
        switch (mode) {
            case GranMode::Pulsar:
                return 4;
            case GranMode::GhostDelay:
                return 1;
            case GranMode::MicroLoop:
                return 3;
            default:
                return 2;
        }
    }

    float sampleRate_ = kDefaultSampleRate;
    float accumulator_ = 0.0f;
};

} // namespace granular
