#include "../s21_string.h"

#include <check.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int s21_test_sign(int value) { return (value > 0) - (value < 0); }

START_TEST(s21_memchr_test) {
  unsigned char data[] = {0, 1, 127, 128, 255, 42, 0};
  ck_assert_ptr_eq(s21_memchr(data, 128, sizeof(data)),
                   memchr(data, 128, sizeof(data)));
  ck_assert_ptr_eq(s21_memchr(data, 511, sizeof(data)),
                   memchr(data, 511, sizeof(data)));
  ck_assert_ptr_null(s21_memchr(data, 7, sizeof(data)));
  ck_assert_ptr_null(s21_memchr(data, 0, 0));
}
END_TEST

START_TEST(s21_memcmp_test) {
  unsigned char first[] = {0, 127, 128, 255, 10};
  unsigned char second[] = {0, 127, 128, 254, 10};
  ck_assert_int_eq(s21_memcmp(first, second, 0), memcmp(first, second, 0));
  ck_assert_int_eq(s21_memcmp(first, first, sizeof(first)), 0);
  ck_assert_int_eq(s21_test_sign(s21_memcmp(first, second, sizeof(first))),
                   s21_test_sign(memcmp(first, second, sizeof(first))));
  ck_assert_int_eq(s21_test_sign(s21_memcmp(second, first, sizeof(first))),
                   s21_test_sign(memcmp(second, first, sizeof(first))));
}
END_TEST

START_TEST(s21_memcpy_test) {
  unsigned char source[] = {0, 1, 2, 128, 255, 42};
  unsigned char expected[12];
  unsigned char actual[12];
  memset(expected, 9, sizeof(expected));
  memset(actual, 9, sizeof(actual));
  ck_assert_ptr_eq(s21_memcpy(actual, source, sizeof(source)), actual);
  memcpy(expected, source, sizeof(source));
  ck_assert_mem_eq(actual, expected, sizeof(actual));
  ck_assert_ptr_eq(s21_memcpy(actual, source, 0), actual);
}
END_TEST

START_TEST(s21_memset_test) {
  unsigned char expected[16];
  unsigned char actual[16];
  memset(expected, 1, sizeof(expected));
  memset(actual, 1, sizeof(actual));
  ck_assert_ptr_eq(s21_memset(actual, 511, 10), actual);
  memset(expected, 511, 10);
  ck_assert_mem_eq(actual, expected, sizeof(actual));
  ck_assert_ptr_eq(s21_memset(actual, 0, 0), actual);
}
END_TEST

START_TEST(s21_strncat_test) {
  char expected[64] = "start:";
  char actual[64] = "start:";
  ck_assert_ptr_eq(s21_strncat(actual, "abcdef", 3), actual);
  strncat(expected, "abcdef", 3);
  ck_assert_str_eq(actual, expected);
  s21_strncat(actual, "", 5);
  strncat(expected, "", 5);
  ck_assert_str_eq(actual, expected);
  s21_strncat(actual, "ignored", 0);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_strchr_test) {
  const char value[] = "abca";
  ck_assert_ptr_eq(s21_strchr(value, 'a'), strchr(value, 'a'));
  ck_assert_ptr_eq(s21_strchr(value, '\0'), strchr(value, '\0'));
  ck_assert_ptr_null(s21_strchr(value, 'z'));
}
END_TEST

START_TEST(s21_strncmp_test) {
  ck_assert_int_eq(s21_strncmp("same", "same", 10), 0);
  ck_assert_int_eq(s21_strncmp("abc", "xyz", 0), 0);
  ck_assert_int_eq(s21_test_sign(s21_strncmp("abc", "abd", 3)),
                   s21_test_sign(strncmp("abc", "abd", 3)));
  ck_assert_int_eq(s21_test_sign(s21_strncmp("abcd", "abc", 10)),
                   s21_test_sign(strncmp("abcd", "abc", 10)));
  ck_assert_int_eq(s21_test_sign(s21_strncmp("abc", "abcd", 10)),
                   s21_test_sign(strncmp("abc", "abcd", 10)));
}
END_TEST

START_TEST(s21_strncpy_test) {
  char expected[16];
  char actual[16];
  memset(expected, '#', sizeof(expected));
  memset(actual, '#', sizeof(actual));
  ck_assert_ptr_eq(s21_strncpy(actual, "abc", 8), actual);
  strncpy(expected, "abc", 8);
  ck_assert_mem_eq(actual, expected, sizeof(actual));
  s21_strncpy(actual, "long text", 4);
  strncpy(expected, "long text", 4);
  ck_assert_mem_eq(actual, expected, sizeof(actual));
  ck_assert_ptr_eq(s21_strncpy(actual, "none", 0), actual);
}
END_TEST

START_TEST(s21_strcspn_test) {
  ck_assert_uint_eq(s21_strcspn("hello", "xyz"), strcspn("hello", "xyz"));
  ck_assert_uint_eq(s21_strcspn("hello", "ol"), strcspn("hello", "ol"));
  ck_assert_uint_eq(s21_strcspn("", "abc"), strcspn("", "abc"));
  ck_assert_uint_eq(s21_strcspn("hello", ""), strcspn("hello", ""));
}
END_TEST

START_TEST(s21_strerror_test) {
  ck_assert_str_eq(s21_strerror(0), strerror(0));
  ck_assert_str_eq(s21_strerror(2), strerror(2));
  ck_assert_str_eq(s21_strerror(-1), strerror(-1));
  ck_assert_str_eq(s21_strerror(INT_MIN), strerror(INT_MIN));
  ck_assert_str_eq(s21_strerror(INT_MAX), strerror(INT_MAX));
}
END_TEST

START_TEST(s21_strlen_test) {
  ck_assert_uint_eq(s21_strlen(""), strlen(""));
  ck_assert_uint_eq(s21_strlen("hello world"), strlen("hello world"));
  ck_assert_uint_eq(s21_strlen("\t\n"), strlen("\t\n"));
}
END_TEST

START_TEST(s21_strpbrk_test) {
  const char value[] = "hello world";
  ck_assert_ptr_eq(s21_strpbrk(value, "od"), strpbrk(value, "od"));
  ck_assert_ptr_null(s21_strpbrk(value, "xyz"));
  ck_assert_ptr_null(s21_strpbrk(value, ""));
  ck_assert_ptr_null(s21_strpbrk("", "abc"));
}
END_TEST

START_TEST(s21_strrchr_test) {
  const char value[] = "abca";
  ck_assert_ptr_eq(s21_strrchr(value, 'a'), strrchr(value, 'a'));
  ck_assert_ptr_eq(s21_strrchr(value, '\0'), strrchr(value, '\0'));
  ck_assert_ptr_null(s21_strrchr(value, 'z'));
}
END_TEST

START_TEST(s21_strstr_test) {
  const char value[] = "abc abcabcd";
  ck_assert_ptr_eq(s21_strstr(value, "abc"), strstr(value, "abc"));
  ck_assert_ptr_eq(s21_strstr(value, "abcabcd"), strstr(value, "abcabcd"));
  ck_assert_ptr_eq(s21_strstr(value, ""), strstr(value, ""));
  ck_assert_ptr_null(s21_strstr(value, "missing"));
  ck_assert_ptr_null(s21_strstr("", "a"));
}
END_TEST

START_TEST(s21_strtok_test) {
  char expected[] = ",,one two,,three,";
  char actual[] = ",,one two,,three,";
  char* expected_token = strtok(expected, ", ");
  char* actual_token = s21_strtok(actual, ", ");
  int processing = 1;
  while (processing) {
    ck_assert_int_eq(expected_token == NULL, actual_token == S21_NULL);
    if (expected_token != NULL && actual_token != S21_NULL) {
      ck_assert_str_eq(actual_token, expected_token);
      expected_token = strtok(NULL, ", ");
      actual_token = s21_strtok(S21_NULL, ", ");
    } else {
      processing = 0;
    }
  }
}
END_TEST

START_TEST(s21_strtok_edge_test) {
  char expected[] = "whole string";
  char actual[] = "whole string";
  ck_assert_str_eq(s21_strtok(actual, ""), strtok(expected, ""));
  ck_assert_ptr_null(s21_strtok(S21_NULL, ""));
  char delimiters_only[] = "::::";
  ck_assert_ptr_null(s21_strtok(delimiters_only, ":"));
}
END_TEST

Suite* s21_string_suite(void) {
  Suite* suite = suite_create("s21_string");
  TCase* test_case = tcase_create("part_1");
  tcase_add_test(test_case, s21_memchr_test);
  tcase_add_test(test_case, s21_memcmp_test);
  tcase_add_test(test_case, s21_memcpy_test);
  tcase_add_test(test_case, s21_memset_test);
  tcase_add_test(test_case, s21_strncat_test);
  tcase_add_test(test_case, s21_strchr_test);
  tcase_add_test(test_case, s21_strncmp_test);
  tcase_add_test(test_case, s21_strncpy_test);
  tcase_add_test(test_case, s21_strcspn_test);
  tcase_add_test(test_case, s21_strerror_test);
  tcase_add_test(test_case, s21_strlen_test);
  tcase_add_test(test_case, s21_strpbrk_test);
  tcase_add_test(test_case, s21_strrchr_test);
  tcase_add_test(test_case, s21_strstr_test);
  tcase_add_test(test_case, s21_strtok_test);
  tcase_add_test(test_case, s21_strtok_edge_test);
  suite_add_tcase(suite, test_case);
  return suite;
}
