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
	printf("なんかおかしい文字を使ってる．英数ひらカタ使え．\n");
	printf("位置: %d byte\n", pos);
	exit(-1);
}

#define LRANGE(X,Y) (X <= low && low <= Y)
#define JA 		ntoffset + ((low % 2) == 1 ? 0x11 : 0x07)
#define JKST	ntoffset + 0x16
#define JN		ntoffset + 0x35
#define JM		ntoffset + 0x3f
#define JR		ntoffset + 0x47
#define JH		ntoffset + 0x3A
#define JWA		ntoffset + 0x4c
#define JWO		ntoffset + 0x06
#define JNN		ntoffset + 0x4d
#define JY		ntoffset + ((low % 2) == 1 ? 0x44 : 0x0c)

char AssignHiraKaku(char low, int hira=1) {
	int ntoffset = (hira == 1 ? 0xc0 : 0x80);
	int typesub = (hira == 1 ? 0x9f : 0x40);	/* ひらカタのテーブル最小値 */
	low -= typesub;		/* 小文字のあからのオフセットに直す */

	/* ひらがな */
	if (LRANGE(0x00,0x08)) {
		/* ひらがなの母音 */
		return low / 2 + JA;
	}
	else if (LRANGE(0x0a,0x28)) {
		/* かさた */
		if (low == 0x22) 
			return ntoffset + 0x0F;			/* 小文字のつは例外 */
		if (0x22 <= low) low--;				/* 小文字のつ以降は自動で繰り下げ */
		return low / 2 + JKST;
	}
	else if (LRANGE(0x29,0x2d)) {
		return low - 0x29 + JN;				/* な */
	}
	else if (LRANGE(0x3d,0x42)) {
		return low - 0x3d + JM;				/* ま */
	}
	else if (LRANGE(0x49,0x4d)) {
		return low - 0x49 + JR;				/* ら */
	}
	else if (LRANGE(0x2e,0x3c)) {
		return (low - 0x2e) / 3 + JH;	/* は */
	}
	else if (LRANGE(0x43,0x48)) {
		return (low - 0x43) / 2 + JY; /* や */
	}
	else if (low == 0x4f) {
		return JWA;	/* わ */
	}
	else if (low == 0x52) {
		return JWO;	/* を */
	}
	else if (low == 0x53) {
		return JNN;	/* ん */
	}
	return -1;
}

/* 濁点は全て無視されるので注意 */
char MultiByteWord(char hi, char low) {
	int pos = 0;
	if (hi == 0x82 && LRANGE(0x9f,0xf1)) {
		return AssignHiraKaku(low);

	} else if (hi == 0x83 && LRANGE(0x40,0x96)) {
		/* カタカナ */
		return AssignHiraKaku(low, 0);

	} else if (hi == 0x89 && low == 0x7e) {
		/* 円だけ例外 */
		return 0x5f;
	}

	return -1;	// なんかおかしいときは-1を返す
}

void ProcParse(char* buffer) {
	int i;
	int cur = 0;
	char* nametable = (char*)malloc(sizeof(buffer));
	for (i = 0; i < sizeof(buffer); i++) {
		if (0x20 <= buffer[i] && buffer[i] <= 0x7d) {
			/* アルファベット処理 */
			nametable[cur] -= 20;
			cur++;
		} else if (0x81 <= buffer[i] && buffer[i] <= 0x9f) {
			/* 2バイト文字の処理 */
			nametable[cur] = MultiByteWord(buffer[i], buffer[++i]);
			if (nametable[cur] == -1)
				AssertionWord(i);
			cur++;
		} else {
			AssertionWord(i);
		}
	}
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

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("引数はファイル名だけ指定してください\n");
		exit(-1);
	}

	MainProc(argv[1]);
	
	return 0;
}