#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

#include "definitions.h"
#include "structs.h"
#include "imageUtils.h"
#include "mpgUtils.h"
#include "bucketUtils.h"
#include "conversion.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


int main(int argc, char* argv[]) {

    // TIMING
    // Record the starting time
    auto startTime = std::chrono::high_resolution_clock::now();
    std::cout << std::fixed << std::setprecision(2);

    std::vector<uint8_t> pic1Data;
    std::vector<uint8_t> pic2Data;
    if (!loadAndResizeImages(argc, argv, pic1Data, pic2Data, startTime)) {
        return -1;
    }

    // Adjusting to be correct format
    std::vector<Pixel> pic1(TOTAL_PIX);
    std::vector<Pixel> pic2(TOTAL_PIX);
    populatePic(pic1Data, pic1);
    populatePic(pic2Data, pic2);
    std::cout << "Successfully converted images to proper types.";
    // TIMING
    auto stopTimeCon = std::chrono::high_resolution_clock::now();
    auto durationCon = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeCon - startTime);
    std::cout << " Program Time: " << (durationCon.count() / 1000000.0f) << " seconds\n\n";

    // Freeing unused vectors
    std::vector<uint8_t>().swap(pic1Data);
    std::vector<uint8_t>().swap(pic2Data);


    // Create the pixel buckets for the images
    // If there is a picture clarity flag change MAX_BUCKET_ERROR to reflect it
    for (uint8_t i = 2; i < (argc - 1); i++) {
        if (std::string_view(argv[i]) == "-b") { MAX_BUCKET_ERROR = std::stof(argv[i + 1]); }
    }
    // Acts as sort of "micro images" allowing the ranking to be much faster
    std::vector<Bucket*> pic1Buckets = createBuckets(pic1);
    std::cout << "Successfully created Pic1's Buckets.\n\n";
    // Assign each pic1 bucket's index
    uint32_t pic1BucketsSize = pic1Buckets.size();
    for (uint32_t i = 0; i < pic1BucketsSize; i++) { pic1Buckets[i]->pic1Index = i; }

    std::vector<Bucket*> pic2Buckets = createBuckets(pic2);
    std::cout << "Successfully created both buckets.";
    // TIMING
    auto stopTimeBuc = std::chrono::high_resolution_clock::now();
    auto durationBuc = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeBuc - startTime);
    std::cout << " Program Time: " << (durationBuc.count() / 1000000.0f) << " seconds\n\n";

    // // ADDITIONAL FUNCTIONALITY
    // //To Flatten Colors: Uncomment this block and Comment out everything below here except the "Placing the new image in the file" block
    // std::vector<Pixel> pic1Trans(TOTAL_PIX);
    // for (Bucket* b : pic1Buckets) {

    //     for (Pixel* p : b->pixels) {
    //         pic1Trans[p->loc].color = static_cast<Colors>(b->avgColor);
    //     }
    // }


    // Rank error to each pixel
    populateRanking(pic1Buckets, pic2Buckets);
    std::cout << "Successfully ranked bucket error.";
    // TIMING
    auto stopTimeRank = std::chrono::high_resolution_clock::now();
    auto durationRank = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeRank - startTime);
    std::cout << " Program Time: " << (durationRank.count() / 1000000.0f) << " seconds\n\n";


    // Create a map to go from pic1Buckets to pic2Buckets
    // Functions as a "hash table" with Pix2 bucket as index and Pix1 bucket as the value
    uint32_t pic2BucketsSize = pic2Buckets.size();
    std::vector<BucketHolder*> bestPic1Map(pic2BucketsSize);
    for (uint32_t i = 0; i < pic2BucketsSize; i++) {

        BucketHolder* tempBHolder = new BucketHolder();

        bestPic1Map[i] = tempBHolder;

        bestPic1Map[i]->spareSpace = pic2Buckets[i]->pixels.size();
    }
    

    mapBuckets(pic1Buckets, bestPic1Map);
    std::cout << "\nSuccessfully mapped buckets.";
    // TIMING
    auto stopTimeMap = std::chrono::high_resolution_clock::now();
    auto durationMap = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeMap - startTime);
    std::cout << " Program Time: " << (durationMap.count() / 1000000.0f) << " seconds\n\n";


    std::vector<Pixel> pic1Trans = reorganizePixels(pic2Buckets, bestPic1Map);
    std::cout << "Successfully reorganized pixels.";
    // TIMING
    auto stopTimeReo = std::chrono::high_resolution_clock::now();
    auto durationReo = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeReo - startTime);
    std::cout << " Program Time: " << (durationReo.count() / 1000000.0f) << " seconds\n\n";


    // Placing the new image in the file
    std::vector<uint8_t> pic1TransData(TOTAL_PIX * CHANNELS);
    compressPic(pic1Trans, pic1TransData, false);

    std::string jpgOutFileName = std::string(argv[3]) + ".jpg";
    if (stbi_write_jpg(jpgOutFileName.c_str(), WIDTH, HEIGHT, CHANNELS, pic1TransData.data(), 90)) {
        std::cout << "Successfully saved converted image.";
    } else {
        std::cerr << "Failed to save converted image.";
    }
    // TIMING
    auto stopTimeSav = std::chrono::high_resolution_clock::now();
    auto durationSav = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeSav - startTime);
    std::cout << " Program Time: " << (durationSav.count() / 1000000.0f) << " seconds\n\n";


    // Create a video showing the conversion
    std::string mpgOutFileName = std::string(argv[3]) + ".mpg";
    createMpegFile(pic1, pic1Trans, mpgOutFileName.c_str());
    std::cout << "Successfully completed.";
    // TIMING
    auto stopTimeMpg = std::chrono::high_resolution_clock::now();
    auto durationMpg = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeMpg - startTime);
    std::cout << " Program Time: " << (durationMpg.count() / 1000000.0f) << " seconds\n\n";

    return 0;
}