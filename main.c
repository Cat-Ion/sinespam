#include <stdint.h>
#include <stdio.h>
#include "synthesis.h"
int main() {
  int32_t buf[128];
  synthesis_init();

  synthesis_set_frequency( 50, 255 );

  for(uint32_t t = 0; t < 48000 * 5; t += 128) {
    synthesis_generate_sound(&buf[0], 128);
    write(1, buf, 128*4);
  }
  return 0;
}
