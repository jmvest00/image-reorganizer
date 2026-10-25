#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include "structs.h"


void populatePic(std::vector<uint8_t>& picData, std::vector<Pixel>& pic);
void compressPic(std::vector<Pixel>& pic, std::vector<uint8_t>& picData, bool isFrame);

#endif // IMAGE_UTILS_H