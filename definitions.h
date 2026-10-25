#ifndef GLOBALS_H
#define GLOBALS_H

#include <cstdint>

#define MAX_PIX_COUNT 524288
extern uint32_t HEIGHT;
extern uint32_t WIDTH;
extern uint32_t TOTAL_PIX;

const int CHANNELS = 3;
#define FPS 24
#define INITIAL_SECONDS 1
#define CONVERSION_SECONDS 3
#define FINAL_SECONDS 1
const uint8_t INITIAL_FRAMES    = FPS * INITIAL_SECONDS;
const uint8_t CONVERSION_FRAMES = FPS * CONVERSION_SECONDS;
const uint8_t FINAL_FRAMES      = FPS * FINAL_SECONDS;

extern float MAX_BUCKET_ERROR;
#define REBUCKETIZE_MAX 10

#endif // GLOBALS_H