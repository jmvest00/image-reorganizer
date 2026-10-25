#ifndef COLOR_UTILS_H
#define COLOR_UTILS_H

#include "structs.h"

// Based off Redmean human sensitivity
float errorOfColor(Colors pix1, Colors pix2);
void recalculateAverage(FloatColors& avgColor, Colors newColor, uint32_t bucketSize, bool isAdding);

#endif // COLOR_UTILS_H