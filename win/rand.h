#pragma once

#include <sodium.h>

static inline uint32_t arc4random() {
    return randombytes_random(); 
}

static inline uint32_t arc4random_uniform(uint32_t upperBound) {
    return randombytes_uniform(upperBound); 
}
