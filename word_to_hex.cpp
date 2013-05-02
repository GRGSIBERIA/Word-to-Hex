#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <memory.h>

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
	fread(buffer, 1, fsize, fp);
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

unsigned char ExceptPattern(unsigned char low, int hira) {
	switch (low) {
		case 0x0b:
			if (hira == 1) return 0x70;	/* �� */
			break;

		case 0x28:
			if (hira == 0) return 0x71;	/* �h */
			break;

		case 0x2F:
			if (hira == 0) return 0x72;	/* �o�@*/
			break;

		case 0x17:
			if (hira == 1) return 0x73;	/* �� */
			break;

		case 0x35:
			if (hira == 1) return 0x74;	/* �� */
			break;

		default:
			return 0xC0;
	}
}

unsigned char AssignHiraKaku(unsigned char low, int hira=1, char* qualify=NULL) {
	int ntoffset = (hira == 1 ? 0xc0 : 0x80);
	int typesub = (hira == 1 ? 0x9f : 0x40);	/* �Ђ�J�^�̃e�[�u���ŏ��l */
	unsigned char except_test;

	if (LRANGE(0x7f,0x96)) low--;	/* �~�ƃ��̊Ԃɋ�Ԃ�����̂ŋl�߂� */
	low -= typesub;								/* �������̂�����̃I�t�Z�b�g�ɒ��� */

	except_test = ExceptPattern(low, hira);
	if (except_test != 0xc0) return except_test;

	if (LRANGE(0x00,0x09)) {
		return low / 2 + JA;	/* �� */
	}
	else if (LRANGE(0x0a,0x28)) {
		/* ������ */
		if (low == 0x22) 
			return ntoffset + 0x0F;			/* �������̂͗�O */
		if (0x22 <= low) low--;				/* �������̂ȍ~�͎����ŌJ�艺�� */
		if ((low + 2) % 2 == 1) {
			if (*qualify == NULL) {				/* ���_ */
				*qualify = 0xbe;
			}
		}
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
		if (*qualify == NULL) {
			switch ((low + 3) % 3) {
				case 2:
					*qualify = 0xbe;
					break;

				case 0:
					*qualify = 0xbf;
					break;
			}
		}
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
	return 0xc0;
}

#define Pattern(X,Y,Z) else if (hi == X && low == Y) { return Z; }

/* ���_�͑S�Ė��������̂Œ��� */
unsigned char MultiByteWord(unsigned char hi, unsigned char low, char* qualify=NULL) {
	int pos = 0;
	if (hi == 0x82 && LRANGE(0x9f,0xf1)) {
		return AssignHiraKaku(low, 1, qualify);			/* �Ђ炪�� */

	} else if (hi == 0x83 && LRANGE(0x40,0x96)) {
		return AssignHiraKaku(low, 0, qualify);	/* �J�^�J�i */

	} 
	Pattern(0x89, 0x7e, 0x5f)	/* �~ */
	Pattern(0x81, 0x60, 0xd0)	/* �g */
	Pattern(0x81, 0x5b, 0x90)	/* ���� */
	Pattern(0x81, 0x40, 0x81)	/* ��_ */
	Pattern(0x81, 0x41, 0x84) /* �Ǔ_ */
	Pattern(0x81, 0x45, 0x85) /* ���� */
	Pattern(0x81, 0x75, 0x82)	/* �u */
	Pattern(0x81, 0x76, 0x83) /* �v */
	Pattern(0x81, 0x79, 0xc2)	/* �y */
	Pattern(0x81, 0x7a, 0xc3)	/* �z */
	Pattern(0x81, 0xf4, 0xc5) /* ���� */
	Pattern(0x81, 0xa1, 0xc4) /* �l�p */
	Pattern(0x81, 0x63, 0xfe)	/* �c�c */
	Pattern(0x81, 0x40, 0x40) /* �S�p�� */

	return 0xc0;	// �Ȃ񂩂��������Ƃ���-1��Ԃ�
}

#define UL(X,Y) (unsigned char)buffer[i] == X && (unsigned char)buffer[i+1] == Y

char* ProcParse(const char* buffer, int length) {
	int i;
	int cur = 0;
	char* nametable = (char*)malloc(length + 1);
	memset(nametable, NULL, length + 1);

	for (i = 0; i < length + 1; i++) {
		if (buffer[i] == '\0') {
			nametable[cur] = 0x00;
			break;

		} else if (buffer[i] == '\n') {
			nametable[cur++] = 0x80;	// 0x80�͉��s�R�[�h�Ȃ̂Ŋo���悤

		} else if ((unsigned char)buffer[i] == 0x20) {
			nametable[cur++] = 0x40;	// 0x40�͗�O�I�ɋ󔒋L���ɂ���

		} else if (UL(0x21,0x3f)) {
			nametable[cur++] = 0xff;
			i++;

		} else if (0x20 <= buffer[i] && buffer[i] <= 0x7d) {
			nametable[cur++] = buffer[i] - 0x20;		/* �A���t�@�x�b�g���� */

		} else if (0x81 <= (unsigned char)buffer[i] && (unsigned char)buffer[i] <= 0x83) {
			/* 2�o�C�g�����̏��� */
			nametable[cur] = MultiByteWord((unsigned char)buffer[i], (unsigned char)buffer[i+1], &nametable[cur+1]);
			if ((unsigned char)nametable[cur] == 0xc0) {
				printf("%02x\n", (unsigned char)buffer[i]);
				AssertionWord(i);
			}
			if (nametable[cur+1] != NULL) cur++;	/* qualify��NULL����Ȃ���΁C���_�����ŃC���N�� */
			cur++;
			i += 1;
		} else {
			AssertionWord(i);
		}
	}

	return nametable;
}

// ���C�������C�@�t�@�C���̓ǂݍ��ݓ�����
const char* MainProc(char* argv) {
	int i;
	char* buffer;
	size_t fsize;
	char* strings;
	FILE* fp = fopen(argv, "r");
	if (fp == NULL) {
		printf("�t�@�C�����J���܂���\n");
		exit(-1);
	}

	fsize = FileSize(fp);
	buffer = ReadFile(fp);
	
	strings = ProcParse(buffer, strlen(buffer));
	for (i = 0; i < fsize; i++) {
		printf("%02x\n", (unsigned char)strings[i]);
	}

	free(buffer);
	fclose(fp);
	return strings;
}

size_t GetBinaryLength(const char* binary) {
	size_t counter = 0;
	for (; binary[counter] != NULL; counter++);
	return counter+1;
}

//#define TEST
#ifndef TEST

int main(int argc, char* argv[]) {
	const char* binary = 0;
	FILE* fp;
	size_t size;

	if (argc > 1) {
		binary = MainProc(argv[1]);
	}

	if (argc == 3) {
		fp = fopen(argv[2], "wb");
	}
	else if (argc == 2) {
		fp = fopen("a.str", "wb");
	}
	else {
		printf("do not match argument.\n");
		exit(-1);
	}

	size = GetBinaryLength(binary);
	printf("%d\n", size);
	fwrite(binary, 1, size, fp);
	fclose(fp);
	
	return 0;
}

#else

void MTest(const char* test, unsigned char answer) {
	unsigned char r = (unsigned char)MultiByteWord((unsigned char)test[0], (unsigned char)test[1]);
	assert(r == answer);
}

void TestStrings(const char* test) {
	int i;
	int terminate = strlen(test);
	char* result;
	printf("test, %d\n", strlen(test));
	result = ProcParse(test, terminate);
	for (i = 0; i < terminate; i++) {
		printf("%02x\n", (unsigned char)result[i]);
	}
	free(result);
}

void TestHiraKaku();
void TestStringDaku();

int main(int argc, char* argv[]) {
	char* result;
	int i;
	//TestHiraKaku();
	TestStringDaku();
	
	return 0;
}

void TestStringDaku() {
	TestStrings("����������");
	TestStrings("����������");
	TestStrings("�����Âł�");
	TestStrings("�_�a�d�f�h");
	TestStrings("�΂тԂׂ�");
	TestStrings("�ς҂Ղ؂�");
	TestStrings("�o�r�u�x�{");
	TestStrings("�p�s�v�y�|");
	TestStrings("!?");
	TestStrings("�����������������������������������ĂƂȂɂʂ˂�");
	TestStrings(" a�@��");
	TestStrings("�u�������v");
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

#endif