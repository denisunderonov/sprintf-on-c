#include <stdlib.h>

#include "s21_string.h"

void* s21_to_upper(const char* str) {
  s21_size_t len = s21_strlen(str);
  char* result = malloc((len + 1) * sizeof(char));

  if (result) {
    for (s21_size_t i = 0; i < len; i++) {
      if (str[i] >= 'a' && str[i] <= 'z') {
        result[i] = str[i] - ('a' - 'A');
      } else {
        result[i] = str[i];
      }
    }

    result[len] = '\0';
  }

  return (void*)result;
}

void* s21_to_lower(const char* str) {
  s21_size_t len = s21_strlen(str);
  char* result = malloc((len + 1) * sizeof(char));

  if (result) {
    for (s21_size_t i = 0; i < len; i++) {
      if (str[i] >= 'A' && str[i] <= 'Z') {
        result[i] = str[i] + ('a' - 'A');
      } else {
        result[i] = str[i];
      }
    }

    result[len] = '\0';
  }

  return (void*)result;
}

void* s21_insert(const char* src, const char* str, s21_size_t start_index) {
  char* result = S21_NULL;

  if (start_index <= s21_strlen(src)) {
    s21_size_t len = s21_strlen(src) + s21_strlen(str);
    result = malloc((len + 1) * sizeof(char));

    if (result) {
      s21_size_t i = 0;
      s21_size_t j = 0;

      for (; i < start_index; i++) {
        result[i] = src[i];
      }

      for (; str[j]; j++) {
        result[i + j] = str[j];
      }

      for (; src[i]; i++) {
        result[i + j] = src[i];
      }

      result[len] = '\0';
    }
  }

  return (void*)result;
}

void* s21_trim(const char* src, const char* trim_chars) {
  const char* trim = S21_NULL;
  char* result = S21_NULL;
  int start = 0;
  int end = s21_strlen(src) - 1;

  if (trim_chars) {
    trim = trim_chars;
  } else {
    trim = " \t\n\v\f\r";
  }

  while (s21_strchr(trim, src[start]) != S21_NULL && src[start]) {
    start++;
  }

  while (end >= start && s21_strchr(trim, src[end]) != S21_NULL) {
    end--;
  }

  if (end >= start) {
    result = malloc((end - start + 2) * sizeof(char));
    if (result) {
      for (int i = 0; i <= end - start; i++) {
        result[i] = src[start + i];
      }

      result[end - start + 1] = '\0';
    }
  } else {
    result = calloc(1, sizeof(char));
  }

  return (void*)result;
}