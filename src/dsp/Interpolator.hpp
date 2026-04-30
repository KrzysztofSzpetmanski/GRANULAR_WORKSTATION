#pragma once

#include <cmath>

namespace granular {

enum class InterpolationMode {
    Nearest = 0,
    Linear,
    CubicHermite
};

class Interpolator {
public:
    static int wrapIndex(int index, int size) {
        if (size <= 0) {
            return 0;
        }
        int wrapped = index % size;
        if (wrapped < 0) {
            wrapped += size;
        }
        return wrapped;
    }

    static float sample(const float* data, int size, float index, InterpolationMode mode) {
        if (!data || size <= 0) {
            return 0.0f;
        }

        switch (mode) {
            case InterpolationMode::Nearest:
                return sampleNearest(data, size, index);
            case InterpolationMode::Linear:
                return sampleLinear(data, size, index);
            case InterpolationMode::CubicHermite:
            default:
                return sampleCubicHermite(data, size, index);
        }
    }

private:
    static float sampleNearest(const float* data, int size, float index) {
        const int i = wrapIndex(static_cast<int>(std::floor(index + 0.5f)), size);
        return data[i];
    }

    static float sampleLinear(const float* data, int size, float index) {
        const int i0 = static_cast<int>(std::floor(index));
        const float t = index - static_cast<float>(i0);
        const float a = data[wrapIndex(i0, size)];
        const float b = data[wrapIndex(i0 + 1, size)];
        return a + (b - a) * t;
    }

    static float sampleCubicHermite(const float* data, int size, float index) {
        const int i1 = static_cast<int>(std::floor(index));
        const float t = index - static_cast<float>(i1);

        const float y0 = data[wrapIndex(i1 - 1, size)];
        const float y1 = data[wrapIndex(i1, size)];
        const float y2 = data[wrapIndex(i1 + 1, size)];
        const float y3 = data[wrapIndex(i1 + 2, size)];

        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * t + c2) * t + c1) * t + c0;
    }
};

} // namespace granular
