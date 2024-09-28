#include <cstdlib>
#include "throw.h"
#include "log.h"
#include "audiolevelprinter.h"

void AudioLevelPrinter::process(const std::vector<int16_t> &buf)
{
	for(std::vector<int16_t>::const_iterator i(buf.begin()); i != buf.end(); ++i) {
		const int16_t absSample(abs(*i));
		if(absSample > m_peak) {
			m_peak = absSample;
		}
	}

	const time_t t(time(NULL));
	if(!m_oldTime) {
		m_oldTime = t;
	}

	if(t != m_oldTime) {
		logn("Peak audio level: %.2f%%", m_peak * 100. / 32768);
		m_peak    = 0;
		m_oldTime = t;
	}
}
