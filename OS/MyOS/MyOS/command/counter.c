#include "stdio.h"

int main(int argc, char * argv[])
{
	int i=(int)((char)(argv[1])-'0');
	for (int m=0; m < 100; m++)
		printf("%d ", m+i*100);
	printf("\n");

	return 0;
}
