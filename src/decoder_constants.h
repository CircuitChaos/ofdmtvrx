#pragma once

namespace decoder_constants {

/* Copied from decoder.hh, keep it in sync or memory corruption will occur! */
static const int spectrum_width = 640, spectrum_height = 64;
// spectrogram_width must be equal to spectrum_width
static const int spectrogram_width = 640, spectrogram_height = 64;
static const int constellation_width = 64, constellation_height = 64;
// constellation_width must be divisible by peak_meter_width
static const int peak_meter_width = 16; //, peak_meter_height = 1;
static const int data_bits        = 43040;

/* Technically these are not decoder constants. They're just here for convenience */
static const unsigned peak_meter_height           = 16;
static const unsigned oscilloscope_initial_width  = 640;
static const unsigned oscilloscope_initial_height = 240;

} // namespace decoder_constants
