#ifndef LED_H
#define LED_H

#include <stdint.h>


typedef struct {
    void (*setBrightness)(uint8_t brightness);
} Led;

void Led_init(Led* led);

#endif