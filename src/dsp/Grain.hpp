#pragma once

namespace granular {

struct Grain {
    bool active = false;
    float posL = 0.0f;
    float posR = 0.0f;
    float step = 1.0f;
    int ageSamples = 0;
    int durationSamples = 1;
    float amp = 0.0f;
    float pan = 0.0f;
    bool reverse = false;
    float texture = 0.5f;
    float lofiHi = 1.0f;
};

} // namespace granular
