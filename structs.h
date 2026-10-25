#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <list>
#include <vector>
#include <cmath>
#include "definitions.h"

struct Ranking {
    
    uint32_t bucketIndex;
    
    float error;

    // Allows for efficient sorting where lowest error is the highest match
    bool operator<(const Ranking& other) const {
        return error > other.error;
    }
};

struct Colors;
struct FloatColors {
    float r;
    float g;
    float b;

    explicit operator Colors() const;
};
struct Colors {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    explicit operator FloatColors() const { return {static_cast<float>(r), static_cast<float>(g), static_cast<float>(b)}; }
};
inline FloatColors::operator Colors() const { return {static_cast<uint8_t>(round(r)), static_cast<uint8_t>(round(g)), static_cast<uint8_t>(round(b))}; }


struct Pixel {

    Colors color;

    uint32_t loc;
};

struct Bucket {

    std::list<Pixel*> pixels;

    FloatColors avgColor;

    
    std::vector<Ranking> rankings;
    uint32_t pic1Index;
};

struct BucketHolder {

    std::vector<Bucket*> buckets;

    uint32_t spareSpace;
};

#endif // TYPES_H