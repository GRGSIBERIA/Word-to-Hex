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

StringList* BinaryToList(const UCHAR* binary, const size_t size) {
  StringList* list = NewStringList();
  size_t i;

  for (i = 0; i < size; i++) Push(list, binary[i]);

  return list;
}

#define TEST
#ifndef TEST

int main(int argc, char* argv[]) {

}

#else

void TestList();
void TestListByBin(StringList*);

int main(int argc, char* argv[]) {
  size_t bin_size;
  UCHAR* binary;
  StringList* list;

  binary = ReadFile(argv[1], &bin_size);
  list = BinaryToList(binary, bin_size);

  TestList();
  TestListByBin(list);

  free(binary);

  return 0;
}

void AccessListAll(StringList* list) {
  int i;
  printf("access list all\n");
  printf("list size: %d\n", list->count);
  for (i = 0; i < list->count; i++) {
    printf("%02x\n", Access(list, i)->value);
  }
}

void TestListByBin(StringList* list) {
  
  AccessListAll(list);
}

void TestList() {
  StringList* list = NewStringList();
  int i;
  StringNode* tmp;
  
  Push(list, 0x01);
  Push(list, 0x02);
  Push(list, 0x03);
  Push(list, 0x04);
  Insert(list, list->_first, 0xFF);
  Insert(list, list->_last, 0xFF);
  
  AccessListAll(list);
}

#endif