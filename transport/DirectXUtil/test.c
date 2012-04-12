#include <stdio.h>

#define MN(a, b, c, d) a ## b ## c ## d
int MN(m, a, i, n)()
{
	printf("hello world\n");
	return 0;
}
