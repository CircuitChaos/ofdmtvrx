#include "decoder_factory.h"
#include "log.h"
#include "throw.h"

DecoderFactory::DecoderFactory(unsigned sampleRate)
{
	/* rate is a template parameter, so we have to do such atrocities here */
	switch(sampleRate) {
		case 8000:
			m_decoder = new Decoder<8000>();
			break;

		case 11025:
			m_decoder = new Decoder<11025>();
			break;

		case 16000:
			m_decoder = new Decoder<16000>();
			break;

		case 22050:
			m_decoder = new Decoder<22050>();
			break;

		case 44100:
			m_decoder = new Decoder<44100>();
			break;

		case 48000:
			m_decoder = new Decoder<48000>();
			break;

		default:
			xthrow("Unsupported sample rate %u Hz, please report bug to have it added", sampleRate);
			break;
	}
}

DecoderFactory::~DecoderFactory()
{
	delete m_decoder;
}

Interface *DecoderFactory::operator()()
{
	return m_decoder;
}
