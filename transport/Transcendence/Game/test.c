#include <stdio.h>

int main()
{
	int i = 5;
	int j = (i = 3) + i;

	printf("%d\n", j);
	return j;
}
