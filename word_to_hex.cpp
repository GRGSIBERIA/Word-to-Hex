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
	printf("なんかおかしい文字を使ってる．英数ひらカタ使え．\n");
	printf("位置: %d byte\n", pos);
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
	int typesub = (hira == 1 ? 0x9f : 0x40);	/* ひらカタのテーブル最小値 */

	if (LRANGE(0x7f,0x96)) low--;	/* ミとムの間に空間がある */
	low -= typesub;		/* 小文字のあからのオフセットに直す */

	/* ひらがな */
	if (LRANGE(0x00,0x09)) {
		return low / 2 + JA;	/* あ */
	}
	else if (LRANGE(0x0a,0x28)) {
		/* かさた */
		if (low == 0x22) 
			return ntoffset + 0x0F;			/* 小文字のつは例外 */
		if (0x22 <= low) low--;				/* 小文字のつ以降は自動で繰り下げ */
		return (low - 0x0a) / 2 + JKST;
	}
	else if (LRANGE(0x29,0x2d)) {
		return low - 0x29 + JN;				/* な */
	}
	else if (LRANGE(0x3d,0x41)) {
		return low - 0x3d + JM;				/* ま */
	}
	else if (LRANGE(0x49,0x4d)) {
		return low - 0x49 + JR;				/* ら */
	}
	else if (LRANGE(0x2e,0x3c)) {
		return (low - 0x2e) / 3 + JH;	/* は */
	}
	else if (LRANGE(0x42,0x48)) {
		return low / 2 + JY - 0x21; /* や */
	}
	else if (low == 0x4e) {
		return JWA;	/* わ */
	}
	else if (low == 0x52) {
		return JNN;	/* ん */
	}
	else if (low == 0x51) {
		return JWO;	/* を */
	}
	return -1;
}

/* 濁点は全て無視されるので注意 */
char MultiByteWord(unsigned char hi, unsigned char low) {
	int pos = 0;
	if (hi == 0x82 && LRANGE(0x9f,0xf1)) {
		return AssignHiraKaku(low);			/* ひらがな */

	} else if (hi == 0x83 && LRANGE(0x40,0x96)) {
		return AssignHiraKaku(low, 0);	/* カタカナ */

	} else if (hi == 0x89 && low == 0x7e) {
		/* 円だけ例外 */
		return 0x5f;
	}

	return -1;	// なんかおかしいときは-1を返す
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
			/* アルファベット処理 */
			nametable[cur++] = buffer[i] - 0x20;

		} else if (0x81 <= buffer[i] && buffer[i] <= 0x9f) {
			/* 2バイト文字の処理 */
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

// メイン処理，　ファイルの読み込み等から
void MainProc(char* argv) {
	int i;
	char* buffer;
	size_t fsize;
	FILE* fp = fopen(argv, "r");
	if (fp == NULL) {
		printf("ファイルが開けません\n");
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
		printf("引数はファイル名だけ指定してください\n");
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
	MTest("ア", 0x91);
	MTest("イ", 0x92);
	MTest("ウ", 0x93);
	MTest("エ", 0x94);
	MTest("オ", 0x95);
	MTest("あ", 0xd1);
	MTest("い", 0xd2);
	MTest("う", 0xd3);
	MTest("え", 0xd4);
	MTest("お", 0xd5);
	MTest("ァ", 0x87);
	MTest("ィ", 0x88);
	MTest("ゥ", 0x89);
	MTest("ェ", 0x8a);
	MTest("ォ", 0x8b);
	MTest("ぁ", 0xc7);
	MTest("ぃ", 0xc8);
	MTest("ぅ", 0xc9);
	MTest("ぇ", 0xca);
	MTest("ぉ", 0xcb);
	MTest("キ", 0x97);
	MTest("き", 0xd7);
	MTest("サ", 0x9b);
	MTest("さ", 0xdb);
	MTest("シ", 0x9c);
	MTest("し", 0xdc);
	MTest("チ", 0xa1);
	MTest("ち", 0xe1);
	MTest("ナ", 0xa5);
	MTest("な", 0xe5);
	MTest("ニ", 0xa6);
	MTest("に", 0xe6);
	MTest("ヒ", 0xab);
	MTest("ひ", 0xeb);
	MTest("ム", 0xb1);
	MTest("む", 0xf1);
	MTest("ヤ", 0xb4);
	MTest("ユ", 0xb5);
	MTest("ヨ", 0xb6);
	MTest("や", 0xf4);
	MTest("ゆ", 0xf5);
	MTest("よ", 0xf6);
	MTest("ワ", 0xbc);
	MTest("ン", 0xbd);
	MTest("わ", 0xfc);
	MTest("ん", 0xfd);
	MTest("ヲ", 0x86);
	MTest("を", 0xc6);
	MTest("円", 0x5f);
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