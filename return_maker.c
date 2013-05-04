#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define UCHAR unsigned char

size_t FileSize(FILE* fp) {
  size_t size;
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  return size;
}

UCHAR* ReadFile(const char* filename, size_t* bin_size) {
  FILE* fp = fopen(filename, "rb");
  *bin_size = FileSize(fp);
  UCHAR* binary = (UCHAR*)malloc(*bin_size);
  fread(binary, sizeof(UCHAR), *bin_size, fp);
  fclose(fp);
  return binary;
}

typedef struct _StringNode {
  struct _StringNode* _next;
  UCHAR value;
} StringNode;

typedef struct _StringList {
  struct _StringNode* _first;
  struct _StringNode* _last;
  int count;
} StringList;

void SetStringNode(StringNode* node, UCHAR value, StringNode* next) {
  node->value = value;
  node->_next = next;
}

StringList* NewStringList() {
  StringList* list = (StringList*)malloc(sizeof(StringList));
  list->count = 0;
  return list;
}

StringNode* NewStringNode() {
  return (StringNode*)malloc(sizeof(StringNode));
}

void Delete(StringList* list, StringNode* target, StringNode* prev) {
  list->count--;
  if (prev != NULL)
    prev->_next = target->_next;
  free(target);
}

void Push(StringList* list, UCHAR val) {
  if (list->count == 0) {
    list->_first = NewStringNode();
    list->_last = list->_first;
    SetStringNode(list->_first, val, NULL);
  } else {
    list->_last->_next = NewStringNode();
    list->_last = list->_last->_next;
    SetStringNode(list->_last, val, NULL);
  }
  list->count++;
}

const StringNode* Access(const StringList* list, const int index) {
  int cnt = 0;
  StringNode* ptr;
  for (ptr = list->_first; ptr != NULL; ptr = ptr->_next) {
    if (cnt++ == index) return ptr;
  }
  return NULL;
}

void Insert(StringList* list, StringNode* node, UCHAR value) {
  StringNode* tmp;
  if (node->_next == NULL) {  /* 末尾に挿入する場合 */
    node->_next = NewStringNode();
    list->_last = node->_next;
    SetStringNode(node->_next, value, NULL);
  } else {
    tmp = node->_next;
    node->_next = NewStringNode();
    SetStringNode(node->_next, value, tmp);
  }
  list->count++;
}

void WriteFile(const char* fname, StringList* list) {
  FILE* fp = fopen(fname, "wb");
  StringNode* ptr = list->_first;
  for (; ptr != NULL; ptr = ptr->_next) {
    fwrite(ptr->value, 1, 1, fp);
  }
  fclose(fp);
}

StringList* BinaryToList(const UCHAR* binary, const size_t size) {
  StringList* list = NewStringList();
  size_t i;

  for (i = 0; i < size; i++) Push(list, binary[i]);

  return list;
}

#define NVAL(X) ptr->_next->_next->value == X

void InsertReturn(StringList* list, const int return_count) {
  int word_count = 1;
  StringNode* ptr;
  for (ptr = list->_first; ptr != NULL; ptr = ptr->_next) {
    printf("wc:%d, %02x\n", word_count, ptr->value);
    if (ptr->value == 0x80) {
      word_count = 0;   /* 改行があったらリセットする */
    }
    else if (word_count >= return_count-2 && ptr->_next != NULL) {
      if (ptr->_next->_next != NULL) {
        if (NVAL(0x81) || NVAL(0x82) || NVAL(0x83) || NVAL(0x84) || NVAL(0xbe) || NVAL(0xbf)) {
          Insert(list, ptr, 0x80);    /* 禁則処理，行頭に来そうな場合， */
          continue;
        }
      }
    }
    if (word_count >= return_count) {
      Insert(list, ptr, 0x80);    /* 通常処理 */
      continue;
    }
    word_count++;
  }
}

/* word_to_hexを実行した直後のエラーコードを除去する */
/* この関数を実行する前に改ページコードはありえない */
void ClearErrorCode(StringList* list) {
  StringNode* ptr = list->_first;
  if (ptr->value == 0xc0) {
    list->_first = ptr->_next;
    Delete(list, ptr, NULL);
    ptr = list->_first;
  }
  for (; ptr->_next != NULL; ptr = ptr->_next) {
    if (ptr->_next->value == 0xc0) {   /* エラーコードの消去 */
      Delete(list, ptr->_next, ptr);
    }
  }
}

/* 改行コードを改ページコードに上書き */
void OverwritePageSkip(StringList* list, const int skip_number) {
  int skip_count = 0;
  StringNode* ptr = list->_first;
  for (; ptr != NULL; ptr = ptr->_next) {
    if (ptr->value == 0x80) {
      if (skip_count >= skip_number-1) {
        skip_count = 0;
        ptr->value = 0xc0;
        continue;
      } else {
        skip_count++;
      }
    }
  }
}

#define TEST
#ifndef TEST

int main(int argc, char* argv[]) {
  char *infile, *outfile, *binary;
  StringList* list;
  size_t bin_size;
  int return_number, page_skip_number;

  if (argc < 5) return -1;

  infile = argv[1];
  outfile = argv[2];
  return_number = atoi(argv[3]);
  page_skip_number = atoi(argv[4]);

  binary = ReadFile(infile, &bin_size);
  list = BinaryToList(binary, bin_size);

  ClearErrorCode(list);
  InsertReturn(list, return_number);
  OverwritePageSkip(list, page_skip_number);

  WriteFile(outfile, list);

  return 0;
}

#else

void AccessListAll(StringList* list) {
  int i;
  printf("access list all\n");
  printf("list size: %d\n", list->count);
  for (i = 0; i < list->count; i++) {
    printf("%02x\n", Access(list, i)->value);
  }
}

void TestListByBin(StringList* list) {
  //AccessListAll(list);
}

void TestList() {
  StringList* list = NewStringList();
  int i;
  StringNode* tmp;
  
  Push(list, 0xc0);
  Push(list, 0x01);
  Push(list, 0xc0);
  Push(list, 0x01);
  Push(list, 0xc0);
  Push(list, 0x01);
  Push(list, 0xc0);
  Push(list, 0x01);
  for (i = 0; i < 20; i++)
    Push(list, 0x01);

  ClearErrorCode(list);
  InsertReturn(list, 3);
  OverwritePageSkip(list, 3);
  
  AccessListAll(list);
}

int main(int argc, char* argv[]) {
  size_t bin_size;
  UCHAR* binary;
  StringList* list;

  //binary = ReadFile(argv[1], &bin_size);
  //list = BinaryToList(binary, bin_size);

  TestList();
  //TestListByBin(list);

  //free(binary);

  return 0;
}

#endif