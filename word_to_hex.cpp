#include <stdio.h>
#include <stdlib.h>

size_t FileSize(FILE* fp) {
	size_t now = ftell(fp);
	size_t last = 0;
	fseek(fp, 0, SEEK_END);
	last = ftell(fp);
	fseek(fp, now, SEEK_SET);
	return last;
}

char* ReadFile(FILE* fp) {
	size_t fsize = FileSize(fp);
	char* buffer = (char*)malloc(fsize);
	fread(buffer, fsize, fsize, fp);
	return buffer;
}

// ���C�������C�@�t�@�C���̓ǂݍ��ݓ�����
void MainProc(char* argv) {
	int i;
	char* buffer;
	size_t fsize;
	FILE* fp = fopen(argv, "r");
	if (fp == NULL) {
		printf("�t�@�C�����J���܂���\n");
		exit(-1);
	}

	fsize = FileSize(fp);
	buffer = ReadFile(fp);
	
	for (i = 0; i < fsize; i++) {
		printf("%c\n", buffer[i]);
	}

	free(buffer);
	fclose(fp);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("�����̓t�@�C���������w�肵�Ă�������\n");
		exit(-1);
	}

	MainProc(argv[1]);
	

	return 0;
}