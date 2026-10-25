#include "imageUtils.h"

void populatePic(std::vector<uint8_t>& picData, std::vector<Pixel>& pic) {

    for (uint32_t i = 0; i < TOTAL_PIX; i++) {

        pic[i].loc = i;

        pic[i].color.r = picData[i * 3];
        pic[i].color.g = picData[i * 3 + 1];
        pic[i].color.b = picData[i * 3 + 2];
    }
}

void compressPic(std::vector<Pixel>& pic, std::vector<uint8_t>& picData, bool isFrame) {

    uint8_t spaceBetweenData = (isFrame ? 4 : 3);

    for (uint32_t i = 0; i < TOTAL_PIX; i++) {

        picData[i * spaceBetweenData]     = pic[i].color.r;
        picData[i * spaceBetweenData + 1] = pic[i].color.g;
        picData[i * spaceBetweenData + 2] = pic[i].color.b;
    }
}