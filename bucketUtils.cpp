#include "bucketUtils.h"

void populateRanking(std::vector<Bucket*>& pic1Buckets, std::vector<Bucket*>& pic2Buckets) {

    // Populating the ranking vector of each pic1 bucket
    uint32_t pic1BucketsSize = pic1Buckets.size();
    uint32_t pic2BucketsSize = pic2Buckets.size();
    for (uint32_t i = 0; i < pic1BucketsSize; i++) {


        Bucket& pic1Bucket = *pic1Buckets[i];
        pic1Bucket.rankings.resize(pic2BucketsSize);
        for (uint32_t j = 0; j < pic2BucketsSize; j++) {

            pic1Bucket.rankings[j] = Ranking{j, errorOfColor(static_cast<Colors>(pic1Bucket.avgColor), static_cast<Colors>(pic2Buckets[j]->avgColor))};
        }

        // Sort by best match
        std::sort(pic1Bucket.rankings.begin(), pic1Bucket.rankings.end());
    }
}

void bucketAssigner(std::vector<Bucket*>& buckets, Pixel* pix) {

    uint32_t bucketsSize = buckets.size();

    uint32_t bestBucketIndex;
    float bestBucketError = MAX_BUCKET_ERROR;
    bool closeEnoughBucketFound = false;
    for (uint32_t i = 0; i < bucketsSize; i++) {

        float error = errorOfColor(pix->color, static_cast<Colors>(buckets[i]->avgColor));

        if (error < bestBucketError) {

            closeEnoughBucketFound = true;
            bestBucketError = error;
            bestBucketIndex = i;
        }
    }
    if (!closeEnoughBucketFound) {

        Bucket* tempBucket = new Bucket();

        tempBucket->pixels.push_back(pix);
        tempBucket->avgColor = static_cast<FloatColors>(pix->color);

        buckets.push_back(tempBucket);
        return;
    }

    // Add it to the proper bucket
    buckets[bestBucketIndex]->pixels.push_back(pix);
    // Update the average
    recalculateAverage(buckets[bestBucketIndex]->avgColor, pix->color, buckets[bestBucketIndex]->pixels.size(), true);
}

std::vector<Bucket*> createBuckets(std::vector<Pixel>& pic) {

    std::vector<Bucket*> buckets;

    // Update the user on percent completion
    std::atomic<uint32_t> currentPixel{0};
    std::atomic<bool> createRunning{true};

    // Launch progress reporter thread (runs every second)
    std::thread progress_thread([&createRunning, &currentPixel]() {
        while (createRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Read atomic value safely
            uint32_t i = currentPixel.load(std::memory_order_relaxed);
            
            std::cout << "Creating Buckets: " << ((i * 100) / TOTAL_PIX) << "%\n" << std::flush;
        }
    });

    // Assign each pixel to a bucket
    for (uint32_t i = 0; i < TOTAL_PIX; i++) {

        bucketAssigner(buckets, &pic[i]);

        currentPixel.store(i, std::memory_order_relaxed);
    }
    createRunning = false;
    progress_thread.join();


    // The buckets are populated but the initial values of each bucket may no longer be within MAX_BUCKET_ERROR
    uint8_t rebucketizeCount = 0;
    while (rebucketizeCount++ < REBUCKETIZE_MAX) {

        std::vector<Pixel*> bucketlessPix;
        // Flag that allows early exit if no pixel was removed from its bucket
        bool changedAnything = false;

        int bucketsSize = buckets.size();
        for (int i = 0; i < bucketsSize; i++) {

            // Look through every pixel in the bucket and remove those that no longer match
            auto pixelsIt = buckets[i]->pixels.begin();
            while (pixelsIt != buckets[i]->pixels.end()) {

                Pixel* pix = *pixelsIt;
                if (errorOfColor(pix->color, static_cast<Colors>(buckets[i]->avgColor)) > MAX_BUCKET_ERROR) {

                    // Update flag
                    changedAnything = true;
                    // Store it
                    bucketlessPix.push_back(pix);

                    // Remove from bucket and get iterator to next element
                    pixelsIt = buckets[i]->pixels.erase(pixelsIt);

                    // Update average (fixed indexing bug to use 'i' and 'pix')
                    if (!buckets[i]->pixels.empty()) { recalculateAverage(buckets[i]->avgColor, pix->color, buckets[i]->pixels.size(), false); }
                } else {
                    pixelsIt++;
                }
            }
            // Remove empty buckets
            if (buckets[i]->pixels.empty()) {
                delete buckets[i];
                buckets.erase(buckets.begin() + i);

                // Shift the for loop to reflect it
                i--;
                bucketsSize--;
            }
        }

        if (!changedAnything) { break; }
        
        uint32_t bucketlessPixSize = bucketlessPix.size();
        for (uint32_t i = 0; i < bucketlessPixSize; i++) {

            bucketAssigner(buckets, bucketlessPix[i]);
        }
    }

    return buckets;
}

// Make a new mini bucket to accomadate a partial match
uint32_t splitIntoMiniBucket(std::vector<Bucket*>& pic1Buckets, uint32_t lookingAtPic1BucketIndex, uint32_t sizeOfMiniBucket) {

    // If there is not enough for everyone put everyone that fits in and keep looking for the rest
    if (sizeOfMiniBucket < pic1Buckets[lookingAtPic1BucketIndex]->pixels.size()) {

        Bucket* miniBucket = new Bucket();

        miniBucket->avgColor = pic1Buckets[lookingAtPic1BucketIndex]->avgColor;

        miniBucket->pixels.splice(
                                miniBucket->pixels.begin(), 
                                pic1Buckets[lookingAtPic1BucketIndex]->pixels, 
                                pic1Buckets[lookingAtPic1BucketIndex]->pixels.begin(), 
                                std::next(pic1Buckets[lookingAtPic1BucketIndex]->pixels.begin(), sizeOfMiniBucket));

        miniBucket->rankings = pic1Buckets[lookingAtPic1BucketIndex]->rankings;
        miniBucket->pic1Index = pic1Buckets[lookingAtPic1BucketIndex]->pic1Index;



        Bucket* remainderBucket = pic1Buckets[lookingAtPic1BucketIndex];
        remainderBucket->pic1Index = pic1Buckets.size();

        pic1Buckets[lookingAtPic1BucketIndex] = miniBucket;
        pic1Buckets.push_back(remainderBucket);

        return remainderBucket->pic1Index;
    }

    return UINT32_MAX;
}

std::array<uint32_t, 2> mapPicBucket(std::vector<Bucket*>& pic1Buckets, std::vector<BucketHolder*>& bestPic1Map, uint32_t lookingAtPic1BucketIndex) {

    // Less verbose aliases
    Bucket* lookingAtPic1Bucket = pic1Buckets[lookingAtPic1BucketIndex];


    // This finds the highest ranked match for lookingAtPic1Bucket
    uint32_t targetPic2Bucket = lookingAtPic1Bucket->rankings.back().bucketIndex;


    // If there is space in the "table" then that is the best spot for everyone that can fit there
    if (bestPic1Map[targetPic2Bucket]->spareSpace >= 1) {

        uint32_t returnIndex = splitIntoMiniBucket(pic1Buckets, lookingAtPic1BucketIndex, bestPic1Map[targetPic2Bucket]->spareSpace);

        bestPic1Map[targetPic2Bucket]->buckets.push_back(pic1Buckets[lookingAtPic1BucketIndex]);
        bestPic1Map[targetPic2Bucket]->spareSpace -= pic1Buckets[lookingAtPic1BucketIndex]->pixels.size();

        return {returnIndex, UINT32_MAX};
    }


    // Otherwise determine which bucket loses the least by moving to it's next best option
    uint32_t pic2BucketHolderSize = bestPic1Map[targetPic2Bucket]->buckets.size();
    uint32_t leastAffectedBucketHolderIndex = UINT32_MAX;
    // Initialized negative so if noone can move the least affected will be lookingAt
    float leastAffectedBucketHolderError = -1;
    for (uint32_t i = 0; i < pic2BucketHolderSize; i++) {
        // Shorter alias
        Bucket* currentPic1Bucket = bestPic1Map[targetPic2Bucket]->buckets[i];

        // Skip buckets that can't move
        if (currentPic1Bucket->rankings.size() < 2) { continue; }

        // Current match
        float currentErrorDif = currentPic1Bucket->rankings.back().error;
        // Next best match
        currentErrorDif -= currentPic1Bucket->rankings[currentPic1Bucket->rankings.size() - 2].error;
        currentErrorDif = std::abs(currentErrorDif);

        // If noone else has been enetered or the previous one is more affected enter this one
        if ((leastAffectedBucketHolderIndex == UINT32_MAX) || (currentErrorDif < leastAffectedBucketHolderError)) {
         
            leastAffectedBucketHolderIndex = i;
            leastAffectedBucketHolderError = currentErrorDif;
        }
    }


    // If lookingAt can't move then someone else MUST move
    float lookingAtErrorDif = -1;
    if (lookingAtPic1Bucket->rankings.size() >= 2) {

        // Find the errorDiff of the lookingAtBucket
        lookingAtErrorDif = lookingAtPic1Bucket->rankings.back().error;
        lookingAtErrorDif -= lookingAtPic1Bucket->rankings[lookingAtPic1Bucket->rankings.size() - 2].error;
        lookingAtErrorDif = std::abs(lookingAtErrorDif);
    }


    // Insert lookingAt and put current in the bucketless
    if (lookingAtErrorDif > leastAffectedBucketHolderError) {

        Bucket* leastAffectedBucket = bestPic1Map[targetPic2Bucket]->buckets[leastAffectedBucketHolderIndex];
        uint32_t leastAffectedBucketIndex = leastAffectedBucket->pic1Index;

        // Make lookingAt small enough to fit in where the pixels were evicted
        uint32_t returnIndex = splitIntoMiniBucket(pic1Buckets, lookingAtPic1BucketIndex, leastAffectedBucket->pixels.size());

        // Only evict the pixels required to be evicted not the whole bucket
        if (returnIndex == UINT32_MAX) {

            uint32_t maintainThisIndex = splitIntoMiniBucket(pic1Buckets, leastAffectedBucket->pic1Index, lookingAtPic1Bucket->pixels.size());

            // If part of the existing bucket is going to remain then keep it around
            if (maintainThisIndex != UINT32_MAX) {

                bestPic1Map[targetPic2Bucket]->buckets.push_back(pic1Buckets[maintainThisIndex]);
            }
        }

        // Switch the new value into the table
        bestPic1Map[targetPic2Bucket]->buckets[leastAffectedBucketHolderIndex] = pic1Buckets[lookingAtPic1BucketIndex];

        // Return the now bucketless value(s)
        return {leastAffectedBucketIndex, returnIndex};

    } else {

        // SAA
        lookingAtPic1Bucket->rankings.pop_back();

        // Return the still bucketless value
        return {lookingAtPic1BucketIndex, UINT32_MAX};
    }
}