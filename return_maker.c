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

const UCHAR* ReadFile(const char* filename, size_t* bin_size) {
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
  return (StringList*)malloc(sizeof(StringList)); 
}

StringNode* NewStringNode() {
  return (StringNode*)malloc(sizeof(StringNode));
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

#define TEST
#ifndef TEST

int main(int argc, char* argv[]) {

}

#else

void TestList();

int main(int argc, char* argv[]) {
  size_t bin_size;
  const UCHAR* binary;

  //binary = ReadFile(argv[1], &bin_size);

  TestList();
  return 0;
}

void TestList() {
  StringList* list = NewStringList();
  int i;
  StringNode* tmp;
  
  Push(list, 0x01);
  Push(list, 0x02);
  Push(list, 0x03);
  Push(list, 0x04);
  
  for (i = 0; i < 4; i++) {
    printf("%02x\n", Access(list, i)->value);
  }
  
}

#endif