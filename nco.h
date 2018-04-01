#ifndef NCO_H
#define NCO_H
#include <stdint.h>

struct NCO {
  uint32_t increment;
  uint32_t phase;
  uint8_t amplitude;
};

void nco_init(float sampling_rate);

void nco_set_frequency(struct NCO *nco, float frequency);
void nco_set_amplitude(struct NCO *nco, uint8_t amplitude);

int32_t nco_tick(struct NCO *nco);
#endif
