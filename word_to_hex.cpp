#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("コマンドライン引数は２つまでです");
	}

	printf("%d\n", argc);
	return 0;
}