#include <portaudio.h>
#include <stdint.h>

#include "nco.h"
#include "synthesis.h"

#define SAMPLING_RATE 48000

static struct NCO ncos[MAXIMUM_HALFTONE];
static int num_on = 0;

void synthesis_init() {
  nco_init(SAMPLING_RATE);
  for(int i = 0; i < MAXIMUM_HALFTONE; i++) {
    // A0 is at 27.5 Hz, use it as reference
    nco_set_frequency(&ncos[i], 27.5 * pow(2., (i - 9.) / 12.));
    nco_set_amplitude(&ncos[i], 0);
  }
}

void synthesis_generate_sound(int32_t *buf, uint32_t num_samples) {
  for(uint32_t i = 0; i < num_samples; i++) {
    int32_t sample = 0;
    for(uint32_t nco = 0; nco < MAXIMUM_HALFTONE; nco++) {
      sample += nco_tick(&ncos[nco]);
    }
    buf[i] = (2 * MAXIMUM_HALFTONE + 1 - 2 * num_on) * sample;
  }
}

int synthesis_set_frequency(int halftone, uint8_t amplitude) {
  if(halftone >= 0 && halftone < MAXIMUM_HALFTONE) {
    struct NCO *nco = &ncos[halftone];
    if(!!nco->amplitude ^ !!amplitude) {
      num_on += amplitude ? 1 : -1;
    }
    nco_set_amplitude(&ncos[halftone], amplitude);
    return 1;
  }
  return 0;
}

int synthesis_get_frequency(int halftone, uint8_t *amplitude) {
  if(halftone >= 0 && halftone < MAXIMUM_HALFTONE) {
    *amplitude = ncos[halftone].amplitude;
    return 1;
  }
  return 0;
}
