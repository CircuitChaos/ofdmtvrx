#include "file.h"

File::File(FILE *fp)
    : m_fp(fp)
{
}

File::~File()
{
	if(m_fp) {
		fclose(m_fp);
	}
}

void File::operator=(FILE *fp)
{
	if(m_fp) {
		fclose(m_fp);
	}

	m_fp = fp;
}

File::operator FILE *()
{
	return m_fp;
}

void File::release()
{
	m_fp = nullptr;
}
