#include <string>
#include <stdexcept>
#include <vector>
#include <cctype>
#include <algorithm>
#include <unistd.h>
#include "throw.h"
#include "log.h"
#include "cli.h"
#include "wav.h"
#include "util.h"
#include "crsdecoder.h"
#include "decoder_constants.h"
#include "decoder_factory.h"
#include "audiolevelprinter.h"

static std::string getModeString(int mode)
{
	struct map {
		int mode;
		std::string text;
	};

	static const map map[] = {
	    {0, "simple ping"},
	    {6, "8PSK, 2700 Hz, 10 s"},
	    {7, "8PSK, 2500 Hz, 11 s"},
	    {8, "QPSK, 2500 Hz, 16 s"},
	    {9, "QPSK, 2250 Hz, 18 s"},
	    {10, "8PSK, 3200 Hz, 9 s"},
	    {11, "8PSK, 2400 Hz, 11 s"},
	    {12, "QPSK, 2400 Hz, 16 s"},
	    {13, "QPSK, 1600 Hz, 24 s"},
	};

	std::string text("unsupported");
	for(size_t i(0); i < sizeof(map) / sizeof(*map); ++i) {
		if(mode == map[i].mode) {
			text = map[i].text;
			break;
		}
	}

	return util::format("%u (%s)", mode, text.c_str());
}

static std::string trimCall(const char *call)
{
	std::string s(std::string(call, 9));
	s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
	return s;
}

static void Main(int argc, char *const argv[])
{
	Cli cli(argc, argv);
	if(cli.needExit()) {
		return;
	}

	logger.setLevel(cli.getLogLevel());
	xassert(!cli.getInputFile().empty() || isatty(0) != 1, "Refusing to read samples from a terminal; use -h for help");

	Wav wav(cli.getInputFile());
	DecoderFactory df(wav.getRate());
	Interface *decoder(df());

	// TODO create output dir (with intermediate subdirs) if doesn't exist
	const std::string outputDir(cli.getOutputDir());
	CRSDecoder crs;

	/* Three constants from decoder, keep in sync with decoder! */
	static const int symbol_length   = (1280 * wav.getRate()) / 8000;
	static const int guard_length    = symbol_length / 8;
	static const int extended_length = symbol_length + guard_length;

	std::vector<int16_t> audioBuffer;
	audioBuffer.resize(extended_length);

	std::vector<uint32_t> spectrum;
	spectrum.resize(decoder_constants::spectrum_width * decoder_constants::spectrum_height);

	std::vector<uint32_t> spectrogram;
	spectrogram.resize(decoder_constants::spectrogram_width * decoder_constants::spectrogram_height);

	std::vector<uint32_t> constellation;
	constellation.resize(decoder_constants::constellation_width * decoder_constants::constellation_height);

	std::vector<uint32_t> peakMeter;
	peakMeter.resize(decoder_constants::peak_meter_width);

	std::vector<uint8_t> payload;
	payload.resize(decoder_constants::data_bits / 8);

	// TODO print audio offset in seconds

	AudioLevelPrinter alp;

	uint32_t offset(0);
	while(wav.getSamples(&audioBuffer[0], audioBuffer.size())) {
		if(cli.getPrintAudioLevel()) {
			alp.process(audioBuffer);
		}

		const int status(decoder->process(&spectrum[0], &spectrogram[0], &constellation[0], &peakMeter[0], &audioBuffer[0], 0, 0));
		switch(status) {
			case STATUS_OKAY:
				break;

			case STATUS_FAIL:
				logv("Error decoding preamble");
				break;

			case STATUS_SYNC: {
				float cfo;
				int32_t mode;
				char call[9];
				decoder->cached(&cfo, &mode, (int8_t *) call);
				logv("Synchronized, carrier: %.2f Hz, mode: %s, call: %s", cfo, getModeString(mode).c_str(), trimCall(call).c_str());
				break;
			}

			case STATUS_DONE: {
				const int bitFlips(decoder->fetch(&payload[0]));
				if(bitFlips < 0) {
					logv("Error decoding data (CRC check failed)");
					break;
				}

				logv("Decoded payload");
				crs.usePayload(payload);
				uint32_t crc32;
				if(crs.fileReady(crc32)) {
					std::string prefix(outputDir);
					if(!prefix.empty()) {
						prefix += "/";
					}

					const std::string fileName(util::format("%08X-%08X.%s", offset, crc32, crs.getExtension().c_str()));
					crs.save(prefix + fileName);
				}
				break;
			}

			/* This actually seems unused */
			case STATUS_HEAP:
				loge("Not enough memory");
				break;

			case STATUS_NOPE: {
				float cfo;
				int32_t mode;
				char call[9];
				decoder->cached(&cfo, &mode, (int8_t *) call);
				logv("%s, carrier: %.2f Hz, mode: %s, call: %s", mode ? "Ignoring preamble" : "Received ping", cfo, getModeString(mode).c_str(), trimCall(call).c_str());
				break;
			}

			default:
				break;
		}

		++offset;
	}
}

int main(int argc, char *const argv[])
{
	try {
		Main(argc, argv);
	}
	catch(const std::runtime_error &e) {
		loge("Fatal error: %s", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
