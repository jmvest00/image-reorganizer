#include "colorUtils.h"

// Based off Redmean human sensitivity
float errorOfColor(Colors pix1, Colors pix2) {
    
    int16_t avgR = (static_cast<int16_t>(pix1.r) + pix2.r) / 2;
    int16_t difR = static_cast<int16_t>(pix1.r) - pix2.r;
    int16_t difG = static_cast<int16_t>(pix1.g) - pix2.g;
    int16_t difB = static_cast<int16_t>(pix1.b) - pix2.b;

    float weightR = 2.0 + (avgR / 256.0);
    float weightG = 4.0;
    float weightB = 2.0 + ((255.0 - avgR) / 256.0);

    return std::sqrt(weightR * difR * difR + weightG * difG * difG + weightB * difB * difB) / 764.833966f;
}

void recalculateAverage(FloatColors& avgColor, Colors newColor, uint32_t bucketSize, bool isAdding) {

    int subtractor = (isAdding ? 1 : -1);

    avgColor.r += subtractor*((newColor.r - avgColor.r) / bucketSize);
    avgColor.g += subtractor*((newColor.g - avgColor.g) / bucketSize);
    avgColor.b += subtractor*((newColor.b - avgColor.b) / bucketSize);
}