#include <stdio.h>

int main()
{
	FILE *f;
	f = fopen("test.out", "w");
	for (int i = 0; i < 100; i++) {
		fwrite(&i, 1, 1, f);
	}
	fclose(f);
	return 0;
}
