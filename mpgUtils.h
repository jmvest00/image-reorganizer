#ifndef MPG_UTILS_H
#define MPG_UTILS_H

#include <vector>
#include <iostream>
#include <cmath>
#include <array>
#include <cstdio>
#include "structs.h"
#include "imageUtils.h"

#include "jo_mpeg.h"

void createMpegFile(std::vector<Pixel>& pic1, std::vector<Pixel>& transPic1, const char* mpgOutFileName);

#endif // MPG_UTILS_H