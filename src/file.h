#pragma once

#include <cstdio>

class File {
public:
	File(FILE *fp = nullptr);
	~File();

	void operator=(FILE *fp);
	operator FILE *();
	void release();

private:
	FILE *m_fp;
};
