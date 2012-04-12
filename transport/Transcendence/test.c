#include <stdio.h>

int main()
{
	int i, acc;

	i = 120;
	acc = ((i << 16) / 7);
	printf("%d 0x%08X\n", acc, acc);
	printf("%d\n", (acc * 7) >> 16);
	return 0;
}
