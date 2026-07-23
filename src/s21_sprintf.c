#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "s21_string.h"
#define S21_BIG_BASE 1000000000U
#define S21_FLOAT_MARGIN 400
typedef struct {
  int flag_minus;
  int flag_plus;
  int flag_space;
  int width;
  int precision;
  int precision_set;
  char length;
  char specifier;
} s21_format;
typedef struct {
  unsigned int* blocks;
  int size;
  int capacity;
} s21_big_uint;
static int s21_is_digit(char symbol);
static void s21_parse_flags(const char* format, int* index,
                            s21_format* options);
static int s21_parse_number(const char* format, int* index);
static s21_format s21_parse_format(const char* format, int* format_index);
static int s21_unsigned_to_string(unsigned long value, char* destination);
static int s21_big_init(s21_big_uint* number, unsigned long long value,
                        int precision);
static void s21_big_multiply(s21_big_uint* number, unsigned int factor);
static int s21_big_divide_two(s21_big_uint* number);
static void s21_big_add_one(s21_big_uint* number);
static int s21_big_to_string(const s21_big_uint* number, char* destination);
static int s21_scaled_float_to_string(long double value, int precision,
                                      char* destination);
static int s21_make_signed(long value, const s21_format* options,
                           char* value_string);
static int s21_make_unsigned(unsigned long value, const s21_format* options,
                             char* value_string);
static int s21_make_float(long double value, const s21_format* options,
                          char* value_string);
static int s21_make_finite_float(long double value, int precision,
                                 char* value_string);
static void s21_write_field(char* str, int* result_length, const char* value,
                            int value_length, const s21_format* options);
static long s21_read_signed(const s21_format* options, va_list* arguments);
static unsigned long s21_read_unsigned(const s21_format* options,
                                       va_list* arguments);
static int s21_string_length(const char* value, const s21_format* options);
static int s21_value_capacity(const s21_format* options);
static void s21_process_format(char* str, int* result_length,
                               const s21_format* options, va_list* arguments);
int s21_sprintf(char* str, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int format_index = 0;
  int result_length = 0;
  while (format[format_index] != '\0') {
    if (format[format_index] != '%') {
      str[result_length++] = format[format_index++];
    } else {
      s21_format options = s21_parse_format(format, &format_index);
      s21_process_format(str, &result_length, &options, &arguments);
    }
  }
  str[result_length] = '\0';
  va_end(arguments);
  return result_length;
}
static int s21_is_digit(char symbol) { return symbol >= '0' && symbol <= '9'; }
static void s21_parse_flags(const char* format, int* index,
                            s21_format* options) {
  int parsing_flags = 1;
  while (parsing_flags) {
    if (format[*index] == '-') {
      options->flag_minus = 1;
      (*index)++;
    } else if (format[*index] == '+') {
      options->flag_plus = 1;
      (*index)++;
    } else if (format[*index] == ' ') {
      options->flag_space = 1;
      (*index)++;
    } else {
      parsing_flags = 0;
    }
  }
}
static int s21_parse_number(const char* format, int* index) {
  int number = 0;
  while (s21_is_digit(format[*index])) {
    number = number * 10 + format[*index] - '0';
    (*index)++;
  }
  return number;
}
static s21_format s21_parse_format(const char* format, int* format_index) {
  s21_format options = {0};
  (*format_index)++;
  s21_parse_flags(format, format_index, &options);
  options.width = s21_parse_number(format, format_index);
  if (format[*format_index] == '.') {
    options.precision_set = 1;
    (*format_index)++;
    options.precision = s21_parse_number(format, format_index);
  }
  if (format[*format_index] == 'h' || format[*format_index] == 'l') {
    options.length = format[*format_index];
    (*format_index)++;
  }
  options.specifier = format[*format_index];
  if (format[*format_index] != '\0') {
    (*format_index)++;
  }
  return options;
}
static int s21_unsigned_to_string(unsigned long value, char* destination) {
  char reversed_digits[32];
  int digit_count = 0;
  do {
    reversed_digits[digit_count++] = (char)('0' + value % 10);
    value /= 10;
  } while (value > 0);
  for (int i = 0; i < digit_count; i++) {
    destination[i] = reversed_digits[digit_count - i - 1];
  }
  destination[digit_count] = '\0';
  return digit_count;
}
static int s21_big_init(s21_big_uint* number, unsigned long long value,
                        int precision) {
  number->capacity = (precision + S21_FLOAT_MARGIN) / 9 + 8;
  number->blocks = calloc((s21_size_t)number->capacity, sizeof(unsigned int));
  number->size = 0;
  if (number->blocks != S21_NULL) {
    do {
      number->blocks[number->size++] = (unsigned int)(value % S21_BIG_BASE);
      value /= S21_BIG_BASE;
    } while (value > 0);
  }
  return number->blocks != S21_NULL;
}
static void s21_big_multiply(s21_big_uint* number, unsigned int factor) {
  unsigned long long carry = 0;
  for (int i = 0; i < number->size; i++) {
    unsigned long long product =
        (unsigned long long)number->blocks[i] * factor + carry;
    number->blocks[i] = (unsigned int)(product % S21_BIG_BASE);
    carry = product / S21_BIG_BASE;
  }
  if (carry > 0 && number->size < number->capacity) {
    number->blocks[number->size++] = (unsigned int)carry;
  }
}
static int s21_big_divide_two(s21_big_uint* number) {
  unsigned long long remainder = 0;
  for (int i = number->size - 1; i >= 0; i--) {
    unsigned long long current = remainder * S21_BIG_BASE + number->blocks[i];
    number->blocks[i] = (unsigned int)(current / 2);
    remainder = current % 2;
  }
  while (number->size > 1 && number->blocks[number->size - 1] == 0) {
    number->size--;
  }
  return (int)remainder;
}
static void s21_big_add_one(s21_big_uint* number) {
  unsigned long long carry = 1;
  int index = 0;
  while (carry > 0 && index < number->size) {
    unsigned long long sum = number->blocks[index] + carry;
    number->blocks[index++] = (unsigned int)(sum % S21_BIG_BASE);
    carry = sum / S21_BIG_BASE;
  }
  if (carry > 0 && number->size < number->capacity) {
    number->blocks[number->size++] = (unsigned int)carry;
  }
}
static int s21_big_to_string(const s21_big_uint* number, char* destination) {
  int top = number->size - 1;
  int length = s21_unsigned_to_string(number->blocks[top], destination);
  for (int block = top - 1; block >= 0; block--) {
    unsigned int divisor = 100000000;
    for (int digit = 0; digit < 9; digit++) {
      destination[length++] =
          (char)('0' + number->blocks[block] / divisor % 10);
      divisor /= 10;
    }
  }
  destination[length] = '\0';
  return length;
}
static int s21_scaled_float_to_string(long double value, int precision,
                                      char* destination) {
  int exponent = 0;
  long double fraction = frexpl(value, &exponent);
  unsigned long long mantissa = (unsigned long long)ldexpl(fraction, 53);
  int shift = exponent - 53 + precision;
  s21_big_uint number;
  int length = 0;
  if (s21_big_init(&number, mantissa, precision)) {
    for (int i = 0; i < precision; i++) s21_big_multiply(&number, 5);
    for (int i = 0; i < shift; i++) s21_big_multiply(&number, 2);
    int lower_bits = 0;
    int half_bit = 0;
    for (int i = 0; i < -shift; i++) {
      int bit = s21_big_divide_two(&number);
      if (i == -shift - 1)
        half_bit = bit;
      else if (bit)
        lower_bits = 1;
    }
    if (half_bit && (lower_bits || number.blocks[0] % 2)) {
      s21_big_add_one(&number);
    }
    length = s21_big_to_string(&number, destination);
    free(number.blocks);
  }
  return length;
}
static int s21_add_integer_sign(char* destination, int negative,
                                const s21_format* options) {
  int length = 0;
  if (negative) {
    destination[length++] = '-';
  } else if (options->flag_plus) {
    destination[length++] = '+';
  } else if (options->flag_space) {
    destination[length++] = ' ';
  }
  return length;
}
static int s21_make_signed(long value, const s21_format* options,
                           char* value_string) {
  int negative = value < 0;
  unsigned long magnitude =
      negative ? 0UL - (unsigned long)value : (unsigned long)value;
  char digits[32];
  int digit_count = s21_unsigned_to_string(magnitude, digits);
  if (options->precision_set && options->precision == 0 && magnitude == 0) {
    digit_count = 0;
  }
  int result_length = s21_add_integer_sign(value_string, negative, options);
  int zero_count = options->precision - digit_count;
  if (zero_count < 0) zero_count = 0;
  for (int i = 0; i < zero_count; i++) value_string[result_length++] = '0';
  for (int i = 0; i < digit_count; i++) {
    value_string[result_length++] = digits[i];
  }
  value_string[result_length] = '\0';
  return result_length;
}
static int s21_make_unsigned(unsigned long value, const s21_format* options,
                             char* value_string) {
  char digits[32];
  int digit_count = s21_unsigned_to_string(value, digits);
  if (options->precision_set && options->precision == 0 && value == 0) {
    digit_count = 0;
  }
  int zero_count = options->precision - digit_count;
  if (zero_count < 0) zero_count = 0;
  int result_length = 0;
  for (int i = 0; i < zero_count; i++) value_string[result_length++] = '0';
  for (int i = 0; i < digit_count; i++) {
    value_string[result_length++] = digits[i];
  }
  value_string[result_length] = '\0';
  return result_length;
}
static int s21_make_float(long double value, const s21_format* options,
                          char* value_string) {
  int result_length = 0;
  int negative = signbit(value) != 0;
  if (negative) value = -value;
  if (isnan(value)) {
#if !defined(__APPLE__) && !defined(__MACH__)
    result_length = s21_add_integer_sign(value_string, negative, options);
#endif
    value_string[result_length++] = 'n';
    value_string[result_length++] = 'a';
    value_string[result_length++] = 'n';
  } else {
    result_length = s21_add_integer_sign(value_string, negative, options);
    if (isinf(value)) {
      value_string[result_length++] = 'i';
      value_string[result_length++] = 'n';
      value_string[result_length++] = 'f';
    } else {
      int precision = options->precision_set ? options->precision : 6;
      result_length +=
          s21_make_finite_float(value, precision, value_string + result_length);
    }
  }
  value_string[result_length] = '\0';
  return result_length;
}
static int s21_make_finite_float(long double value, int precision,
                                 char* value_string) {
  char* scaled = malloc((s21_size_t)precision + S21_FLOAT_MARGIN);
  int scaled_length = 0;
  int result_length = 0;
  if (scaled != S21_NULL) {
    scaled_length = s21_scaled_float_to_string(value, precision, scaled);
    if (precision > 0 && scaled_length <= precision) {
      value_string[result_length++] = '0';
      value_string[result_length++] = '.';
      for (int i = scaled_length; i < precision; i++) {
        value_string[result_length++] = '0';
      }
    }
    for (int i = 0; i < scaled_length; i++) {
      if (precision > 0 && scaled_length > precision &&
          i == scaled_length - precision) {
        value_string[result_length++] = '.';
      }
      value_string[result_length++] = scaled[i];
    }
    free(scaled);
  }
  value_string[result_length] = '\0';
  return result_length;
}
static void s21_write_field(char* str, int* result_length, const char* value,
                            int value_length, const s21_format* options) {
  int padding = options->width - value_length;
  if (padding < 0) padding = 0;
  if (!options->flag_minus) {
    for (int i = 0; i < padding; i++) str[(*result_length)++] = ' ';
  }
  for (int i = 0; i < value_length; i++) {
    str[(*result_length)++] = value[i];
  }
  if (options->flag_minus) {
    for (int i = 0; i < padding; i++) str[(*result_length)++] = ' ';
  }
}
static void s21_process_format(char* str, int* result_length,
                               const s21_format* options, va_list* arguments) {
  if (options->specifier == 's') {
    const char* value = va_arg(*arguments, const char*);
    int length = s21_string_length(value, options);
    s21_write_field(str, result_length, value, length, options);
  } else {
    int capacity = s21_value_capacity(options);
    char* value_string = malloc((s21_size_t)capacity);
    int value_length = 0;
    if (value_string != S21_NULL) {
      if (options->specifier == '%') {
        value_string[value_length++] = '%';
      } else if (options->specifier == 'c') {
        value_string[value_length++] = (char)va_arg(*arguments, int);
      } else if (options->specifier == 'd') {
        long value = s21_read_signed(options, arguments);
        value_length = s21_make_signed(value, options, value_string);
      } else if (options->specifier == 'u') {
        unsigned long value = s21_read_unsigned(options, arguments);
        value_length = s21_make_unsigned(value, options, value_string);
      } else if (options->specifier == 'f') {
        long double value = (long double)va_arg(*arguments, double);
        value_length = s21_make_float(value, options, value_string);
      }
      value_string[value_length] = '\0';
      s21_write_field(str, result_length, value_string, value_length, options);
      free(value_string);
    }
  }
}
static long s21_read_signed(const s21_format* options, va_list* arguments) {
  long value;
  if (options->length == 'l') {
    value = va_arg(*arguments, long);
  } else if (options->length == 'h') {
    value = (short)va_arg(*arguments, int);
  } else {
    value = va_arg(*arguments, int);
  }
  return value;
}
static unsigned long s21_read_unsigned(const s21_format* options,
                                       va_list* arguments) {
  unsigned long value;
  if (options->length == 'l') {
    value = va_arg(*arguments, unsigned long);
  } else if (options->length == 'h') {
    value = (unsigned short)va_arg(*arguments, unsigned int);
  } else {
    value = va_arg(*arguments, unsigned int);
  }
  return value;
}
static int s21_string_length(const char* value, const s21_format* options) {
  int length = 0;
  int has_room = !options->precision_set || length < options->precision;
  while (value[length] != '\0' && has_room) {
    length++;
    has_room = !options->precision_set || length < options->precision;
  }
  return length;
}
static int s21_value_capacity(const s21_format* options) {
  int capacity = 34;
  if (options->specifier == 'f') {
    int precision = options->precision_set ? options->precision : 6;
    capacity = precision + S21_FLOAT_MARGIN;
  } else if ((options->specifier == 'd' || options->specifier == 'u') &&
             options->precision + 3 > capacity) {
    capacity = options->precision + 3;
  }
  return capacity;
}
