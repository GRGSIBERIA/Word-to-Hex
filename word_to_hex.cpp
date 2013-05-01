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

void AssertionWord(int pos) {
	printf("�Ȃ񂩂��������������g���Ă�D�p���Ђ�J�^�g���D\n");
	printf("�ʒu: %d byte\n", pos);
	exit(-1);
}

char Small() {
	
}

char MultiByteWord(char hi, char low) {
	if (hi == 0x82 && 0x9f <= low && low <= 0xf1) {
		/* �Ђ炪�� */
	} else if (hi == 0x83 && 0x40 <= low && low <= 0x96) {
		/* �J�^�J�i */
	} else if (hi == 0x89 && low == 0x7e) {
		/* �~������O */
		return 5f;
	}

	return -1;	// �Ȃ񂩂��������Ƃ���-1��Ԃ�
}

void ProcParse(char* buffer) {
	int i;
	int cur = 0;
	char* nametable = (char*)malloc(sizeof(buffer));
	for (i = 0; i < sizeof(buffer); i++) {
		if (0x20 <= buffer[i] && buffer[i] <= 0x7d) {
			/* �A���t�@�x�b�g���� */
			nametable[cur] -= 20;
			cur++;
		} else if (0x81 <= buffer[i] && buffer[i] <= 0x9f) {
			/* 2�o�C�g�����̏��� */
			nametable[cur] = MultiByteWord(buffer[i], buffer[++i]);
			if (nametable[cur] == -1)
				AssertionWord(i);
			cur++;
		} else {
			AssertionWord(i);
		}
	}
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
	
	ProcParse(buffer);

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