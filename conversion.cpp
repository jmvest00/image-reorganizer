#include "conversion.h"

bool loadAndResizeImages(int argc, char* argv[], 
                         std::vector<uint8_t>& pic1Data, 
                         std::vector<uint8_t>& pic2Data, 
                         std::chrono::high_resolution_clock::time_point startTime) {
    // Loading images
    int pic1Width, pic1Height, pic1Channels;
    uint8_t* pic1DataRaw = stbi_load(argv[1], &pic1Width, &pic1Height, &pic1Channels, CHANNELS);
    if (pic1DataRaw == nullptr) {
        std::cerr << "Failed to load image 1: " << stbi_failure_reason() << "\n";
        return false;
    }

    int pic2Width, pic2Height, pic2Channels;
    uint8_t* pic2DataRaw = stbi_load(argv[2], &pic2Width, &pic2Height, &pic2Channels, CHANNELS);
    if (pic2DataRaw == nullptr) {
        std::cerr << "Failed to load image 2: " << stbi_failure_reason() << "\n";
        return false;
    }
    std::cout << "Successfully loaded images.";
    // TIMING
    auto stopTimeLoad = std::chrono::high_resolution_clock::now();
    auto durationLoad = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeLoad - startTime);
    std::cout << " Program Time: " << (durationLoad.count() / 1000000.0f) << " seconds\n\n";


    // Resizing images
    float aspectRatio = static_cast<float>(pic1Width) / pic1Height;
    float scaleFactor = 1;
    uint32_t goalTOTAL_PIX = 0;
    bool exactDimensionsSet = false;

    // If there are special dimensions parameters use those
    std::string dimensions = "NONE";
    for (uint8_t i = 2; i < (argc - 1); i++) {
        if (std::string_view(argv[i]) == "-d") { dimensions = argv[i + 1]; }
    }
    if (dimensions != "NONE") {
        uint8_t xPos;
        // If the user input pixel counts use those
        if ((xPos = dimensions.find("x")) != std::string::npos || (xPos = dimensions.find("X")) != std::string::npos) {

            WIDTH = std::stoi(dimensions.substr(0, xPos));
            HEIGHT = std::stoi(dimensions.substr(xPos + 1, dimensions.size()));

            if (WIDTH <= 0 || HEIGHT <= 0) {
                std::cerr << (WIDTH <= 0 ? "Width" : "Height") << " must be greater than 0.\n";
                return false;
            }

            TOTAL_PIX = static_cast<uint32_t>(WIDTH) * HEIGHT;

            if (TOTAL_PIX <= MAX_PIX_COUNT) {
                exactDimensionsSet = true;
            } else {
                aspectRatio = static_cast<float>(WIDTH) / HEIGHT;
                goalTOTAL_PIX = MAX_PIX_COUNT;
            }

        // Otherwise use the provided scale such that 2.0 means an image with 2x as many pixels
        } else {
            scaleFactor = (std::stof(dimensions));

            if (scaleFactor <= 0) {
                std::cerr << "Scale factor must be greater than 0.\n";
                return false;
            }
        }
    }

    // Calculate WIDTH, HEIGHT, and TOTAL_PIX from aspect ratio and goal pixel total if needed
    if (!exactDimensionsSet) {
        if (goalTOTAL_PIX == 0) { goalTOTAL_PIX = std::min(static_cast<uint32_t>(pic1Width * pic1Height * scaleFactor), static_cast<uint32_t>(MAX_PIX_COUNT)); }
        WIDTH = static_cast<uint32_t>(std::round(std::sqrt(goalTOTAL_PIX * aspectRatio)));
        HEIGHT = static_cast<uint32_t>(std::round(WIDTH / aspectRatio));
        TOTAL_PIX = WIDTH * HEIGHT;
    }

    // Resize Pic1 and Pic2 to fit the new parameters
    pic1Data.resize(TOTAL_PIX * CHANNELS);
    stbir_resize_uint8_linear(
        pic1DataRaw, pic1Width, pic1Height, 0,
        pic1Data.data(), WIDTH, HEIGHT, 0,
        STBIR_RGB
    );
    stbi_image_free(pic1DataRaw);

    pic2Data.resize(TOTAL_PIX * CHANNELS);
    stbir_resize_uint8_linear(
        pic2DataRaw, pic2Width, pic2Height, 0,
        pic2Data.data(), WIDTH, HEIGHT, 0,
        STBIR_RGB
    );
    stbi_image_free(pic2DataRaw);
    std::cout << "Successfully resized images.";
    // TIMING
    auto stopTimeRes = std::chrono::high_resolution_clock::now();
    auto durationRes = std::chrono::duration_cast<std::chrono::microseconds>(stopTimeRes - startTime);
    std::cout << " Program Time: " << (durationRes.count() / 1000000.0f) << " seconds\n\n";

    return true;
}

void mapBuckets(std::vector<Bucket*>& pic1Buckets,
                           std::vector<BucketHolder*>& bestPic1Map) {
    // Fill the "table" with the best fit for each Pic1Bucket
    std::queue<uint32_t> lookingAtPic1BucketIndices;
    
    uint32_t pic1BucketsSizeV2 = pic1Buckets.size();
    for (uint32_t i = 0; i < pic1BucketsSizeV2; i++) { lookingAtPic1BucketIndices.push(i); }


    // Create a thread that gives feedback to the user that mapping is still being worked on
    std::atomic<bool> mappingDone{false};

    // Spin up background progress thread
    std::thread progress_thread([&mappingDone]() {

        std::cout << "Mapping buckets: ";

        while (!mappingDone) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (!mappingDone) {
                std::cout << "X" << std::flush;
            }
        }
    });

    // "Recursively" continues until we can advance to the next Pic1Bucket (without overflowing stack)
    while (!lookingAtPic1BucketIndices.empty()) {

        // Do the element that most long ago got returned
        std::array<uint32_t, 2> returnedIndices = mapPicBucket(pic1Buckets, bestPic1Map, lookingAtPic1BucketIndices.front());
        lookingAtPic1BucketIndices.pop();

        // Only push non UINT32_MAX elements into the queue
        for (uint8_t i = 0; i < 2; i++) {
            if (returnedIndices[i] == UINT32_MAX) { break; }
            lookingAtPic1BucketIndices.push(returnedIndices[i]);
        }
    }
    mappingDone = true;
    progress_thread.join();
}

std::vector<Pixel> reorganizePixels(std::vector<Bucket*>& pic2Buckets, 
                             std::vector<BucketHolder*>& bestPic1Map) {
    // Reorganize the pixels
    std::random_device random;
    std::mt19937 randomSeed(random());

    // Shuffle pixels such that we don't have distinct bands of similar color
    uint32_t bestPic1MapSize = bestPic1Map.size();
    std::vector<std::vector<Pixel*>> shuffledCondensedBuckets(bestPic1MapSize);
    for (uint32_t i = 0; i < bestPic1MapSize; i++) {

        shuffledCondensedBuckets[i].reserve(pic2Buckets[i]->pixels.size());

        uint32_t bucketsCount = bestPic1Map[i]->buckets.size();
        for (uint32_t j = 0; j < bucketsCount; j++) {
            
            shuffledCondensedBuckets[i].insert(shuffledCondensedBuckets[i].end(), bestPic1Map[i]->buckets[j]->pixels.begin(), bestPic1Map[i]->buckets[j]->pixels.end());
        }

        std::shuffle(shuffledCondensedBuckets[i].begin(), shuffledCondensedBuckets[i].end(), randomSeed);
    }

    // Add the shuffled colors to the final image
    std::vector<Pixel> pic1Trans(TOTAL_PIX);
    for (uint32_t i = 0; i < bestPic1MapSize; i++) {

        auto pic2PixelsIt = pic2Buckets[i]->pixels.begin();

        for (Pixel* pS : shuffledCondensedBuckets[i]) {
            
            pic1Trans[(*(pic2PixelsIt++))->loc] = *pS;
        }
    }

    return pic1Trans;
}