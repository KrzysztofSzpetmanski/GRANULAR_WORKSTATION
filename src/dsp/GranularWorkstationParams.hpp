#pragma once

namespace granular {

enum class GranMode {
    CloudRev = 0,
    GhostDelay,
    FreezeSpace,
    TimeStretch,
    PitchShift,
    MicroLoop,
    Pulsar
};

constexpr int kMaxGrains = 48;
constexpr float kBufferSeconds = 4.0f;
constexpr float kDefaultSampleRate = 48000.0f;
constexpr int kRingBufferSamples = static_cast<int>(kBufferSeconds * kDefaultSampleRate);

struct GranularWorkstationParams {
    GranMode mode = GranMode::CloudRev;

    float mix = 0.5f;
    float size = 0.2f;
    float texture = 0.5f;
    float density = 0.4f;
    float overlap = 0.5f;
    float positionSpread = 0.5f;
    float pitch = 0.0f;
    float reverseProb = 0.0f;
    float space = 0.5f;
    float feedback = 0.3f;
    float damp = 0.6f;
    float reverb = 0.5f;

    bool freeze = false;
    bool clockTick = false;
};

} // namespace granular
