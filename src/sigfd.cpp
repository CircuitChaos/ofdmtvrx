#include <csignal>
#include <sys/signalfd.h>
#include <unistd.h>
#include <cerrno>
#include "throw.h"
#include "log.h"
#include "sigfd.h"

SigFD::SigFD()
{
	xassert(sigemptyset(&m_sigset) == 0, "sigemptyset(): %m");
	xassert(sigaddset(&m_sigset, SIGTERM) == 0, "sigaddset(SIGTERM): %m");
	xassert(sigaddset(&m_sigset, SIGINT) == 0, "sigaddset(SIGINT): %m");
	xassert(sigaddset(&m_sigset, SIGUSR1) == 0, "sigaddset(SIGUSR1): %m");

	m_fd = signalfd(-1, &m_sigset, 0);
	xassert(m_fd >= 0, "signalfd(): %m");

	if(sigprocmask(SIG_BLOCK, &m_sigset, nullptr) != 0) {
		const int se(errno);
		close(m_fd);
		errno = se;
		xthrow("sigprocmask(): %m");
	}
}

SigFD::~SigFD()
{
	close(m_fd);
	sigprocmask(SIG_UNBLOCK, &m_sigset, nullptr);
}

int SigFD::getFD() const
{
	return m_fd;
}

int SigFD::readHandler()
{
	signalfd_siginfo si;
	const ssize_t rs(read(m_fd, &si, sizeof(si)));
	xassert(rs != 0, "read(): EOF when reading from signalfd");
	xassert(rs > 0, "read(): error reading from signalfd: %m");
	xassert((size_t) rs == sizeof(si), "read(): size mismatch reading from signalfd: %zd ne %zu", rs, sizeof(si));
	return si.ssi_signo;
}
