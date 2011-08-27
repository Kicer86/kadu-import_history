#ifndef crc32_h
#define crc32_h

#include <QtGlobal>

class Conv
{
	public:
		static quint32 crc32 (char *buf, int len);
};

#endif
