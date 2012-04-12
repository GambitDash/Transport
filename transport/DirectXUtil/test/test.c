#include <stdio.h>

int main()
{
	unsigned int x = 0;
	unsigned int y = 0xff;

	int a, b, c, d, e, f, i;

	a = x - y; 
	a = a << 16;
	a = a / 299;
	printf("%x\n", a);
	b = y << 16;
	printf("%x\n", b);
	for (i = 0; i < 7; i++) {
		b = b + a;
		printf("%x - %d\n", b, b >> 16);

	}
	return 0;
}

