// Memset implementation

#include "memset.h"

void *memset(void *s, int c, size_t n)
{
   // TODO: IMPLEMENT THIS FUNCTION
	unsigned char* p=s;
   	while(n--)
   		*p++ = (unsigned char)c;
   	return s;
}
