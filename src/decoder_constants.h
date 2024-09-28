#pragma once

namespace decoder_constants {

/* Copied from decoder.hh, keep it in sync or memory corruption will occur! */
static const int spectrum_width = 640, spectrum_height = 64;
static const int spectrogram_width = 640, spectrogram_height = 64;
static const int constellation_width = 64, constellation_height = 64;
static const int peak_meter_width = 16; //, peak_meter_height = 1;
static const int data_bits        = 43040;

} // namespace decoder_constants
