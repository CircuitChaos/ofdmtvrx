#pragma once

#include <csignal>

class SigFD {
public:
	SigFD();
	~SigFD();

	int getFD() const;
	void readHandler();

private:
	sigset_t m_sigset;
	int m_fd{-1};
};
