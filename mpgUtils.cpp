#include "mpgUtils.h"

void createMpegFile(std::vector<Pixel>& pic1, std::vector<Pixel>& transPic1, const char* mpgOutFileName) {

    FILE *mpgOutFile = fopen(mpgOutFileName, "wb");
    if (!mpgOutFile) {
        std::cerr << "Failed to open output file: " << mpgOutFileName << std::endl;
        return;
    }


    std::vector<uint8_t> frameBuffer(TOTAL_PIX * 4);    // RGB + UNUSED ALPHA

    // Add initial pix1 frames
    compressPic(pic1, frameBuffer, true);
    for (uint8_t i = 0; i < INITIAL_FRAMES; i++) {
        jo_write_mpeg(mpgOutFile, frameBuffer.data(), WIDTH, HEIGHT, FPS);
    }


    // Calculate how fast each pixel needs to move to get there in time
    const float PIXEL_VELO       = std::sqrt((WIDTH * WIDTH) + (HEIGHT * HEIGHT)) / CONVERSION_FRAMES;

    std::vector<std::array<float, 2>> velocityMap(TOTAL_PIX);       // For each pixel: 0 of the array is xVelo, 1 of the array is yVelo
    std::vector<uint8_t> doneFrameMap(TOTAL_PIX);                   // Only used by same velo
    std::vector<uint32_t> bestPix1Reverse(TOTAL_PIX);               // Takes in pix1 and outputs it's final location
    for (uint32_t i = 0; i < TOTAL_PIX; i++) {

        uint32_t initialIndex = transPic1[i].loc;
        uint16_t initialX = initialIndex % WIDTH;
        uint16_t initialY = initialIndex / WIDTH;

        uint16_t finalX = i % WIDTH;
        uint16_t finalY = i / WIDTH;


        bestPix1Reverse[initialIndex] = i;
        

        // All pixels move at same speed
        // Delta X and Delta Y calcs
        float deltaX = (static_cast<float>(finalX) - initialX);
        float deltaY = (static_cast<float>(finalY) - initialY);
        float totalDistance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));
        // Protect from divide by 0
        if (totalDistance == 0) {
            velocityMap[initialIndex][0] = 0;
            velocityMap[initialIndex][1] = 0;
            doneFrameMap[initialIndex] = 0;
        } else {
            velocityMap[initialIndex][0] = (deltaX / totalDistance) * PIXEL_VELO;
            velocityMap[initialIndex][1] = (deltaY / totalDistance) * PIXEL_VELO;
            doneFrameMap[initialIndex] = totalDistance / PIXEL_VELO;
        }
        
    }

    // Add each moving frame by calculating where each pixel needs to move to
    std::vector<Pixel> frameBufferPixels = pic1;
    for (uint8_t i = 1; i < CONVERSION_FRAMES; i++) {

        // Creates 1 frame by adding each pixel in it's new spot
        for (uint32_t j = 0; j < TOTAL_PIX; j++) {

            // If it's already in the right spot leave it there
            if (doneFrameMap[j] <= i) {
                frameBufferPixels[bestPix1Reverse[j]] = pic1[j];
            } else {

                int32_t newX = std::round((j % WIDTH) + (velocityMap[j][0] * i));
                int32_t newY = std::round((j / WIDTH) + (velocityMap[j][1] * i));

                if (newX < 0)               { newX = 0; }
                else if(newX >= static_cast<int32_t>(WIDTH))  { newX = WIDTH - 1; }
                if (newY < 0)               { newY = 0; }
                else if(newY >= static_cast<int32_t>(HEIGHT))  { newY = HEIGHT - 1; }

                frameBufferPixels[newX + (newY * WIDTH)] = pic1[j];
            }
        }
        compressPic(frameBufferPixels, frameBuffer, true);

        jo_write_mpeg(mpgOutFile, frameBuffer.data(), WIDTH, HEIGHT, FPS);

        if (!(i % 7)) {
            std::cout << "Intermediary Frame: " << ((i * 100) / CONVERSION_FRAMES) << "%\n";
        }
    }


    // Add final transformed pix1 frames
    compressPic(transPic1, frameBuffer, true);
    for (uint8_t i = 0; i < FINAL_FRAMES; i++) {
        jo_write_mpeg(mpgOutFile, frameBuffer.data(), WIDTH, HEIGHT, FPS);
    }


    // Close the file
    fclose(mpgOutFile);
}