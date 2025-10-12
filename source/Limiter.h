//------------------------------------------------------------------------
// Copyright(c) 2025 Anis Dadou (GullDSP)
//------------------------------------------------------------------------

#include <math.h>
/// <summary>
/// A limiter which is completely linear up to the threshold, 
/// after this follows a tanh waveshaping function
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="threshold"></param>
/// <returns></returns>
static inline float getLimitedSample(float x, float threshold = 0.99) {

    float T = threshold;
    // [TODO] Replace tanh with more efficient method
    if (x > T) {
        float w = (x - T) / (1.0 - T);

        return T + tanh(w) * (1.0 - T);
    }
    if (x < (-1.0 * T)) {
        float w = (-x - T) / (1.0 - T);

        return -T - tanh(w) * (1.0 - T);
    }

    return x;
}