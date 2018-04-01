#include <math.h>
#include "nco.h"

#define RESOLUTION_LOG 10
#define RESOLUTION (1<<RESOLUTION_LOG)

static int16_t waveform[RESOLUTION];
static float nco_sampling_rate = 1;

void nco_init(float sampling_rate) {
  nco_sampling_rate = sampling_rate;

  for(int i = 0; i < RESOLUTION; i++) {
    waveform[i] = sin(2 * M_PI * i / RESOLUTION) * 32767;
  }
}

void nco_set_frequency(struct NCO *nco, float frequency) {
  nco->increment = frequency * pow(2, 32) / nco_sampling_rate;
}

void nco_set_amplitude(struct NCO *nco, uint8_t amplitude) {
  nco->amplitude = amplitude;
}

int32_t nco_tick(struct NCO *nco) {
  nco->phase += nco->increment;
  int32_t retval = nco->amplitude;
  retval *= waveform[nco->phase >> (32 - RESOLUTION_LOG)];
  return retval;
}

