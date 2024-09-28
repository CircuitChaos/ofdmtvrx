#pragma once

#include <cassert>
#include <cstring>
#include "assempix/decoder.hh"

class DecoderFactory {
public:
	DecoderFactory(unsigned sampleRate);
	~DecoderFactory();

	Interface *operator()();

private:
	Interface *m_decoder{nullptr};
};
