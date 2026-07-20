#include "s21_string.h"

void* s21_memchr(const void* str, int c, s21_size_t n) {
  int success = 0;
  void* result = S21_NULL;

  const unsigned char* ptr = (const unsigned char*)str;

  for (s21_size_t i = 0; i < n && !success; i++) {
    if (ptr[i] == (unsigned char)c) {
      success = 1;
      result = (void*)&ptr[i];
    }
  }

  return result;
}

int s21_memcmp(const void* str1, const void* str2, s21_size_t n) {
  int success = 1;
  int result = 0;

  const unsigned char* ptr1 = (const unsigned char*)str1;
  const unsigned char* ptr2 = (const unsigned char*)str2;

  for (s21_size_t i = 0; i < n && success; i++) {
    if (ptr1[i] != ptr2[i]) {
      success = 0;
      result = (int)ptr1[i] - (int)ptr2[i];
    }
  }

  return result;
}

void* s21_memcpy(void* dest, const void* src, s21_size_t n) {
  unsigned char* ptr_dest = (unsigned char*)dest;
  const unsigned char* ptr_src = (const unsigned char*)src;

  for (s21_size_t i = 0; i < n; i++) {
    ptr_dest[i] = ptr_src[i];
  }

  return dest;
}

void* s21_memset(void* str, int c, s21_size_t n) {
  unsigned char ch = (unsigned char)c;
  unsigned char* ptr = (unsigned char*)str;

  for (s21_size_t i = 0; i < n; i++) {
    ptr[i] = ch;
  }

  return str;
}