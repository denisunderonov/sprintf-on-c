#include <check.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../s21_string.h"

Suite* s21_string_suite(void);

START_TEST(s21_sprintf_text_percent_test) {
  char expected[256];
  char actual[256];
  int expected_count = sprintf(expected, "text %% %c %s", 'A', "line");
  int actual_count = s21_sprintf(actual, "text %% %c %s", 'A', "line");
  ck_assert_str_eq(actual, expected);
  ck_assert_int_eq(actual_count, expected_count);
}
END_TEST

START_TEST(s21_sprintf_char_width_test) {
  char expected[256];
  char actual[256];
  sprintf(expected, "[%5c][%-5c]", 'X', 'Y');
  s21_sprintf(actual, "[%5c][%-5c]", 'X', 'Y');
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_string_format_test) {
  char expected[256];
  char actual[256];
  sprintf(expected, "[%10.4s][%-8.3s][%.0s]", "abcdef", "hello", "text");
  s21_sprintf(actual, "[%10.4s][%-8.3s][%.0s]", "abcdef", "hello", "text");
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_long_string_test) {
  char source[3001];
  char expected[3200];
  char actual[3200];
  for (int i = 0; i < 3000; i++) source[i] = (char)('a' + i % 26);
  source[3000] = '\0';
  sprintf(expected, "%s", source);
  s21_sprintf(actual, "%s", source);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_signed_test) {
  char expected[512];
  char actual[512];
  sprintf(expected, "[%d][%+8d][% 8d][%-8d][%8.5d][%.0d]", -42, 42, 42, -42, 42,
          0);
  s21_sprintf(actual, "[%d][%+8d][% 8d][%-8d][%8.5d][%.0d]", -42, 42, 42, -42,
              42, 0);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_signed_length_test) {
  char expected[512];
  char actual[512];
  sprintf(expected, "%hd %hd %ld %ld", (int)SHRT_MIN, (int)SHRT_MAX, LONG_MIN,
          LONG_MAX);
  s21_sprintf(actual, "%hd %hd %ld %ld", (int)SHRT_MIN, (int)SHRT_MAX, LONG_MIN,
              LONG_MAX);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_unsigned_test) {
  char expected[512];
  char actual[512];
  sprintf(expected, "[%u][%8.5u][%-8u][%.0u][%hu][%lu]", 42U, 42U, 42U, 0U,
          (unsigned int)USHRT_MAX, ULONG_MAX);
  s21_sprintf(actual, "[%u][%8.5u][%-8u][%.0u][%hu][%lu]", 42U, 42U, 42U, 0U,
              (unsigned int)USHRT_MAX, ULONG_MAX);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_float_test) {
  char expected[1024];
  char actual[1024];
  sprintf(expected, "[%f][%.2f][%+10.3f][% 10.0f][%-12.4f]", 1.25, 1.236, 0.125,
          2.5, -12.5);
  s21_sprintf(actual, "[%f][%.2f][%+10.3f][% 10.0f][%-12.4f]", 1.25, 1.236,
              0.125, 2.5, -12.5);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_float_special_test) {
  char expected[512];
  char actual[512];
  sprintf(expected, "[%f][%f][%+f][%f]", -0.0, INFINITY, NAN, -INFINITY);
  s21_sprintf(actual, "[%f][%f][%+f][%f]", -0.0, INFINITY, NAN, -INFINITY);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_float_limit_test) {
  char expected[1024];
  char actual[1024];
  sprintf(expected, "%.2f %.6f", DBL_MAX, DBL_MIN);
  s21_sprintf(actual, "%.2f %.6f", DBL_MAX, DBL_MIN);
  ck_assert_str_eq(actual, expected);
}
END_TEST

START_TEST(s21_sprintf_large_precision_test) {
  char expected[6000];
  char actual[6000];
  sprintf(expected, "%.2200d %.2200f", 7, 1.0);
  s21_sprintf(actual, "%.2200d %.2200f", 7, 1.0);
  ck_assert_str_eq(actual, expected);
}
END_TEST

Suite* s21_sprintf_suite(void) {
  Suite* suite = suite_create("s21_sprintf");
  TCase* test_case = tcase_create("core");
  tcase_add_test(test_case, s21_sprintf_text_percent_test);
  tcase_add_test(test_case, s21_sprintf_char_width_test);
  tcase_add_test(test_case, s21_sprintf_string_format_test);
  tcase_add_test(test_case, s21_sprintf_long_string_test);
  tcase_add_test(test_case, s21_sprintf_signed_test);
  tcase_add_test(test_case, s21_sprintf_signed_length_test);
  tcase_add_test(test_case, s21_sprintf_unsigned_test);
  tcase_add_test(test_case, s21_sprintf_float_test);
  tcase_add_test(test_case, s21_sprintf_float_special_test);
  tcase_add_test(test_case, s21_sprintf_float_limit_test);
  tcase_add_test(test_case, s21_sprintf_large_precision_test);
  suite_add_tcase(suite, test_case);
  return suite;
}

int main(void) {
  Suite* suite = s21_sprintf_suite();
  SRunner* runner = srunner_create(suite);
  srunner_add_suite(runner, s21_string_suite());
  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);
  srunner_free(runner);
  return failed == 0 ? 0 : 1;
}
