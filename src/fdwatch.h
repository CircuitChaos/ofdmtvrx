#pragma once

#include <set>

class FDWatch {
public:
	void add(int fd);
	void watch();
	bool isReadable(int fd) const;

private:
	std::set<int> m_infds;
	std::set<int> m_outfds;
	int m_numfds{0};
};
