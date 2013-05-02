#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

#define LRANGE(X,Y) (X <= low && low <= Y)
#define JA 		ntoffset + (((low + 2) % 2) == 1 ? 0x11 : 0x07)
#define JKST	ntoffset + 0x16
#define JN		ntoffset + 0x25
#define JM		ntoffset + 0x2f
#define JR		ntoffset + 0x37
#define JH		ntoffset + 0x2A
#define JWA		ntoffset + 0x3c
#define JWO		ntoffset + 0x06
#define JNN		ntoffset + 0x3d
#define JY		ntoffset + (((low + 2) % 2) == 1 ? 0x34 : 0x0d)

char AssignHiraKaku(unsigned char low, int hira=1) {
	int ntoffset = (hira == 1 ? 0xc0 : 0x80);
	int typesub = (hira == 1 ? 0x9f : 0x40);	/* �Ђ�J�^�̃e�[�u���ŏ��l */

	if (LRANGE(0x7f,0x96)) low--;	/* �~�ƃ��̊Ԃɋ�Ԃ����� */
	low -= typesub;		/* �������̂�����̃I�t�Z�b�g�ɒ��� */

	/* �Ђ炪�� */
	if (LRANGE(0x00,0x09)) {
		return low / 2 + JA;	/* �� */
	}
	else if (LRANGE(0x0a,0x28)) {
		/* ������ */
		if (low == 0x22) 
			return ntoffset + 0x0F;			/* �������̂͗�O */
		if (0x22 <= low) low--;				/* �������̂ȍ~�͎����ŌJ�艺�� */
		return (low - 0x0a) / 2 + JKST;
	}
	else if (LRANGE(0x29,0x2d)) {
		return low - 0x29 + JN;				/* �� */
	}
	else if (LRANGE(0x3d,0x41)) {
		return low - 0x3d + JM;				/* �� */
	}
	else if (LRANGE(0x49,0x4d)) {
		return low - 0x49 + JR;				/* �� */
	}
	else if (LRANGE(0x2e,0x3c)) {
		return (low - 0x2e) / 3 + JH;	/* �� */
	}
	else if (LRANGE(0x42,0x48)) {
		return low / 2 + JY - 0x21; /* �� */
	}
	else if (low == 0x4e) {
		return JWA;	/* �� */
	}
	else if (low == 0x52) {
		return JNN;	/* �� */
	}
	else if (low == 0x51) {
		return JWO;	/* �� */
	}
	return -1;
}

/* ���_�͑S�Ė��������̂Œ��� */
char MultiByteWord(unsigned char hi, unsigned char low) {
	int pos = 0;
	if (hi == 0x82 && LRANGE(0x9f,0xf1)) {
		return AssignHiraKaku(low);			/* �Ђ炪�� */

	} else if (hi == 0x83 && LRANGE(0x40,0x96)) {
		return AssignHiraKaku(low, 0);	/* �J�^�J�i */

	} else if (hi == 0x89 && low == 0x7e) {
		/* �~������O */
		return 0x5f;
	}

	return -1;	// �Ȃ񂩂��������Ƃ���-1��Ԃ�
}

char* ProcParse(const char* buffer) {
	int i;
	int cur = 0;
	char* nametable = (char*)malloc(sizeof(buffer));
	for (i = 0; i < sizeof(buffer); i++) {
		if (buffer[i] == '\0') {
			nametable[cur] = 0;
			break;

		} else if (buffer[i] == '\n') {
			continue;

		} else if (0x20 <= buffer[i] && buffer[i] <= 0x7d) {
			/* �A���t�@�x�b�g���� */
			nametable[cur++] = buffer[i] - 0x20;

		} else if (0x81 <= buffer[i] && buffer[i] <= 0x9f) {
			/* 2�o�C�g�����̏��� */
			nametable[cur] = MultiByteWord((unsigned char)buffer[i], (unsigned char)buffer[++i]);
			if (nametable[cur] == -1)
				AssertionWord(i);
			cur++;
		} else {
			AssertionWord(i);
		}
	}
	return nametable;
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

#define TEST

#ifndef TEST

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("�����̓t�@�C���������w�肵�Ă�������\n");
		exit(-1);
	}

	MainProc(argv[1]);
	
	return 0;
}

#else

void MTest(const char* test, unsigned char answer) {
	unsigned char r = (unsigned char)MultiByteWord((unsigned char)test[0], (unsigned char)test[1]);
	assert(r == answer);
}

void TestHiraKaku() {
	MTest("�A", 0x91);
	MTest("�C", 0x92);
	MTest("�E", 0x93);
	MTest("�G", 0x94);
	MTest("�I", 0x95);
	MTest("��", 0xd1);
	MTest("��", 0xd2);
	MTest("��", 0xd3);
	MTest("��", 0xd4);
	MTest("��", 0xd5);
	MTest("�@", 0x87);
	MTest("�B", 0x88);
	MTest("�D", 0x89);
	MTest("�F", 0x8a);
	MTest("�H", 0x8b);
	MTest("��", 0xc7);
	MTest("��", 0xc8);
	MTest("��", 0xc9);
	MTest("��", 0xca);
	MTest("��", 0xcb);
	MTest("�L", 0x97);
	MTest("��", 0xd7);
	MTest("�T", 0x9b);
	MTest("��", 0xdb);
	MTest("�V", 0x9c);
	MTest("��", 0xdc);
	MTest("�`", 0xa1);
	MTest("��", 0xe1);
	MTest("�i", 0xa5);
	MTest("��", 0xe5);
	MTest("�j", 0xa6);
	MTest("��", 0xe6);
	MTest("�q", 0xab);
	MTest("��", 0xeb);
	MTest("��", 0xb1);
	MTest("��", 0xf1);
	MTest("��", 0xb4);
	MTest("��", 0xb5);
	MTest("��", 0xb6);
	MTest("��", 0xf4);
	MTest("��", 0xf5);
	MTest("��", 0xf6);
	MTest("��", 0xbc);
	MTest("��", 0xbd);
	MTest("��", 0xfc);
	MTest("��", 0xfd);
	MTest("��", 0x86);
	MTest("��", 0xc6);
	MTest("�~", 0x5f);
}

int main(int argc, char* argv[]) {
	char* result;
	int i;
	TestHiraKaku();

	printf("test\n");
	result = ProcParse("!#abc");
	for (i = 0; i < sizeof(result); i++) {
		printf("%02x\n", (unsigned char)result[i]);
	}
	free(result);
	return 0;
}

#endif