#ifndef BUCKET_UTILS_H
#define BUCKET_UTILS_H

#include <vector>
#include <array>
#include <queue>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "structs.h"
#include "colorUtils.h"

void bucketAssigner(std::vector<Bucket*>& buckets, Pixel* pix);
std::vector<Bucket*> createBuckets(std::vector<Pixel>& pic);
void populateRanking(std::vector<Bucket*>& pic1Buckets, std::vector<Bucket*>& pic2Buckets);
uint32_t splitIntoMiniBucket(std::vector<Bucket*>& pic1Buckets, uint32_t lookingAtPic1BucketIndex, uint32_t sizeOfMiniBucket);
std::array<uint32_t, 2> mapPicBucket(std::vector<Bucket*>& pic1Buckets, std::vector<BucketHolder*>& bestPic1Map, uint32_t lookingAtPic1BucketIndex);

#endif // BUCKET_UTILS_H