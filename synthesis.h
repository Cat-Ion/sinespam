struct frequency_data {
  float amplitude;
  float phase_at_zero;
} 

const int maximum_halftone = 7 * 12;

void set_frequency(int halftone, struct frequency_data const *data);
bool get_frequency(int halftone, struct frequency_data *data);

