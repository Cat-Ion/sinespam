#ifndef SYNTHESIS_H
#define SYNTHESIS_H
struct frequency_data {
  float amplitude;
};

#define SAMPLING_RATE 48000
#define MAXIMUM_HALFTONE (7*12)

void synthesis_init(void);

void synthesis_generate_sound(int32_t *buf, uint32_t num_samples);

int synthesis_set_frequency(int halftone, uint8_t amplitude);
int synthesis_get_frequency(int halftone, uint8_t *amplitude);
#endif
