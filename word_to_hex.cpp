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

unsigned char ExceptPattern(unsigned char low, int hira) {
	switch (low) {
		case 0x0b:
			if (hira == 1) return 0x70;	/* が */
			break;

		case 0x28:
			if (hira == 0) return 0x71;	/* ド */
			break;

		case 0x2F:
			if (hira == 0) return 0x72;	/* バ　*/
			break;

		case 0x17:
			if (hira == 1) return 0x73;	/* じ */
			break;

		case 0x35:
			if (hira == 1) return 0x74;	/* ぶ */
			break;

		default:
			return 0xC0;
	}
}

unsigned char AssignHiraKaku(unsigned char low, int hira=1, char* qualify=NULL) {
	int ntoffset = (hira == 1 ? 0xc0 : 0x80);
	int typesub = (hira == 1 ? 0x9f : 0x40);	/* ひらカタのテーブル最小値 */
	unsigned char except_test;

	if (LRANGE(0x7f,0x96)) low--;	/* ミとムの間に空間があるので詰める */
	low -= typesub;								/* 小文字のあからのオフセットに直す */

	except_test = ExceptPattern(low, hira);
	if (except_test != 0xc0) return except_test;

	if (LRANGE(0x00,0x09)) {
		return low / 2 + JA;	/* あ */
	}
	else if (LRANGE(0x0a,0x28)) {
		/* かさた */
		if (low == 0x22) 
			return ntoffset + 0x0F;			/* 小文字のつは例外 */
		if (0x22 <= low) low--;				/* 小文字のつ以降は自動で繰り下げ */
		if ((low + 2) % 2 == 1) {
			if (*qualify == NULL) {				/* 濁点 */
				*qualify = 0xbe;
			}
		}
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
	return 0xc0;
}

#define Pattern(X,Y,Z) else if (hi == X && low == Y) { return Z; }

/* 濁点は全て無視されるので注意 */
unsigned char MultiByteWord(unsigned char hi, unsigned char low, char* qualify=NULL) {
	int pos = 0;
	if (hi == 0x82 && LRANGE(0x9f,0xf1)) {
		return AssignHiraKaku(low, 1, qualify);			/* ひらがな */

	} else if (hi == 0x83 && LRANGE(0x40,0x96)) {
		return AssignHiraKaku(low, 0, qualify);	/* カタカナ */

	} 
	Pattern(0x89, 0x7e, 0x5f)	/* 円 */
	Pattern(0x81, 0x60, 0xd0)	/* 波 */
	Pattern(0x81, 0x5b, 0x90)	/* 長音 */
	Pattern(0x81, 0x40, 0x81)	/* 句点 */
	Pattern(0x81, 0x41, 0x84) /* 読点 */
	Pattern(0x81, 0x45, 0x85) /* 中黒 */
	Pattern(0x81, 0x75, 0x82)	/* 「 */
	Pattern(0x81, 0x76, 0x83) /* 」 */
	Pattern(0x81, 0x79, 0xc2)	/* 【 */
	Pattern(0x81, 0x7a, 0xc3)	/* 】 */
	Pattern(0x81, 0xf4, 0xc5) /* 音符 */
	Pattern(0x81, 0xa1, 0xc4) /* 四角 */
	Pattern(0x81, 0x63, 0xfe)	/* …… */
	Pattern(0x81, 0x40, 0x40) /* 全角空白 */

	return 0xc0;	// なんかおかしいときは-1を返す
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
			nametable[cur++] = 0x80;	// 0x80は改行コードなので覚えよう

		} else if ((unsigned char)buffer[i] == 0x20) {
			nametable[cur++] = 0x40;	// 0x40は例外的に空白記号にする

		} else if (UL(0x21,0x3f)) {
			nametable[cur++] = 0xff;
			i++;

		} else if (0x20 <= buffer[i] && buffer[i] <= 0x7d) {
			nametable[cur++] = buffer[i] - 0x20;		/* アルファベット処理 */

		} else if (0x81 <= (unsigned char)buffer[i] && (unsigned char)buffer[i] <= 0x83) {
			/* 2バイト文字の処理 */
			nametable[cur] = MultiByteWord((unsigned char)buffer[i], (unsigned char)buffer[i+1], &nametable[cur+1]);
			if ((unsigned char)nametable[cur] == 0xc0) {
				printf("%02x\n", (unsigned char)buffer[i]);
				AssertionWord(i);
			}
			if (nametable[cur+1] != NULL) cur++;	/* qualifyがNULLじゃなければ，濁点扱いでインクリ */
			cur++;
			i += 1;
		} else {
			AssertionWord(i);
		}
	}

	return nametable;
}

// メイン処理，　ファイルの読み込み等から
const char* MainProc(char* argv) {
	int i;
	char* buffer;
	size_t fsize;
	char* strings;
	FILE* fp = fopen(argv, "r");
	if (fp == NULL) {
		printf("ファイルが開けません\n");
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
	TestStrings("がぎぐげご");
	TestStrings("ざじずぜぞ");
	TestStrings("だぢづでど");
	TestStrings("ダヂヅデド");
	TestStrings("ばびぶべぼ");
	TestStrings("ぱぴぷぺぽ");
	TestStrings("バビブベボ");
	TestStrings("パピプペポ");
	TestStrings("!?");
	TestStrings("あいうえおかきくけこさしすせそたちつてとなにぬねの");
	TestStrings(" a　あ");
	TestStrings("「あいう」");
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

#endif