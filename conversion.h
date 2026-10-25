#ifndef CONVERSION_H
#define CONVERSION_H

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <atomic>
#include <queue>
#include <random>

#include "definitions.h"
#include "structs.h"
#include "colorUtils.h"
#include "bucketUtils.h"

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

bool loadAndResizeImages(int argc, char* argv[], 
                         std::vector<uint8_t>& pic1Data, 
                         std::vector<uint8_t>& pic2Data, 
                         std::chrono::high_resolution_clock::time_point startTime);

void mapBuckets(std::vector<Bucket*>& pic1Buckets,
                           std::vector<BucketHolder*>& bestPic1Map);

std::vector<Pixel> reorganizePixels(std::vector<Bucket*>& pic2Buckets, 
                             std::vector<BucketHolder*>& bestPic1Map);

#endif // CONVERSION_H