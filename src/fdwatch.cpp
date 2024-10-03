#include <sys/select.h>
#include "fdwatch.h"
#include "log.h"
#include "throw.h"

void FDWatch::add(int fd)
{
	m_infds.insert(fd);
	m_numfds = fd;
	for(std::set<int>::const_iterator i(m_infds.begin()); i != m_infds.end(); ++i) {
		if(*i > m_numfds) {
			m_numfds = *i;
		}
	}

	++m_numfds;
}

void FDWatch::watch()
{
	fd_set rfds;
	FD_ZERO(&rfds);
	for(std::set<int>::const_iterator i(m_infds.begin()); i != m_infds.end(); ++i) {
		FD_SET(*i, &rfds);
	}

	const int rs(select(m_numfds, &rfds, nullptr, nullptr, nullptr));
	xassert(rs != 0, "select(): zero return, but no imeout set");
	xassert(rs > 0, "select(): %m");

	m_outfds.clear();
	for(std::set<int>::const_iterator i(m_infds.begin()); i != m_infds.end(); ++i) {
		if(FD_ISSET(*i, &rfds)) {
			m_outfds.insert(*i);
		}
	}
}

bool FDWatch::isReadable(int fd) const
{
	return m_outfds.find(fd) != m_outfds.end();
}
