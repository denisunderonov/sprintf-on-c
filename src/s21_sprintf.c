#include <math.h>    // frexpl, ldexpl, signbit, isnan и isinf для %f.
#include <stdarg.h>  // va_list, va_start, va_arg и va_end для аргументов ...
#include <stdlib.h>  // malloc, calloc и free для временных буферов.

#include "s21_string.h"  // Объявление s21_sprintf, s21_size_t и S21_NULL.

// Основание системы хранения большого целого: один блок содержит 9 цифр.
#define S21_BIG_BASE 1000000000U
// Запас памяти для преобразования вещественного числа в десятичную строку.
#define S21_FLOAT_MARGIN 400

// Все параметры, прочитанные между символом '%' и спецификатором.
typedef struct {
  int flag_minus;   // 1 для флага '-': выравнивание по левому краю.
  int flag_plus;    // 1 для флага '+': знак выводится и у положительных чисел.
  int flag_space;   // 1 для флага ' ': пробел перед положительным числом.
  int width;        // Минимальная ширина всего поля.
  int precision;    // Точность: цифры после точки или минимум цифр целого.
  int precision_set;  // Отличает заданную точность .0 от её отсутствия.
  char length;      // Модификатор типа: 'h', 'l' или нулевой символ.
  char specifier;   // Тип преобразования: c, s, d, u, f или %.
} s21_format;

// Неотрицательное число произвольного размера для точного форматирования %f.
typedef struct {
  unsigned int* blocks;  // Массив блоков по основанию S21_BIG_BASE.
  int size;              // Количество используемых блоков.
  int capacity;          // Количество выделенных блоков.
} s21_big_uint;

// Ниже объявлены внутренние функции, недоступные из других файлов.
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

// Формирует строку str по строке format и возвращает число записанных символов.
int s21_sprintf(char* str, const char* format, ...) {
  va_list arguments;             // Объект для последовательного чтения ...
  va_start(arguments, format);   // Начинаем читать аргументы после format.
  int format_index = 0;          // Позиция текущего символа в format.
  int result_length = 0;         // Текущая длина и позиция записи в str.

  // Просматриваем формат до завершающего нулевого символа.
  while (format[format_index] != '\0') {
    // Обычный символ не требует форматирования: просто копируем его.
    if (format[format_index] != '%') {
      str[result_length++] = format[format_index++];
    } else {
      // После '%' читаем флаги, ширину, точность, длину и спецификатор.
      s21_format options = s21_parse_format(format, &format_index);
      // Извлекаем нужный аргумент, преобразуем его и записываем в str.
      s21_process_format(str, &result_length, &options, &arguments);
    }
  }
  str[result_length] = '\0';  // Любая C-строка должна заканчиваться '\0'.
  va_end(arguments);          // Завершаем работу со списком аргументов.
  return result_length;       // '\0' не входит в возвращаемую длину.
}

// Проверяет, является ли символ десятичной цифрой.
static int s21_is_digit(char symbol) { return symbol >= '0' && symbol <= '9'; }

// Последовательно читает поддерживаемые флаги '-', '+' и пробел.
static void s21_parse_flags(const char* format, int* index,
                            s21_format* options) {
  int parsing_flags = 1;  // Пока равен 1, следующий символ может быть флагом.
  while (parsing_flags) {
    if (format[*index] == '-') {
      options->flag_minus = 1;  // Включаем выравнивание влево.
      (*index)++;               // Переходим к следующему символу формата.
    } else if (format[*index] == '+') {
      options->flag_plus = 1;  // Требуем явный знак положительного числа.
      (*index)++;              // Пропускаем прочитанный '+'.
    } else if (format[*index] == ' ') {
      options->flag_space = 1;  // Требуем пробел перед положительным числом.
      (*index)++;               // Пропускаем прочитанный пробел.
    } else {
      parsing_flags = 0;  // Первый не-флаг завершает этот этап разбора.
    }
  }
}

// Читает целое неотрицательное число из format, начиная с *index.
static int s21_parse_number(const char* format, int* index) {
  int number = 0;  // Здесь постепенно накапливается результат.
  while (s21_is_digit(format[*index])) {
    // Сдвигаем прежнее число на один десятичный разряд и добавляем цифру.
    number = number * 10 + format[*index] - '0';
    (*index)++;  // Переходим к следующей цифре или части спецификации.
  }
  return number;  // Если цифр не было, возвращается 0.
}

// Разбирает одну спецификацию формата, например "%-10.3ld".
static s21_format s21_parse_format(const char* format, int* format_index) {
  s21_format options = {0};  // По умолчанию все параметры выключены.
  (*format_index)++;         // Пропускаем символ '%'.

  // Формат разбирается в стандартном порядке: флаги, ширина, точность...
  s21_parse_flags(format, format_index, &options);
  options.width = s21_parse_number(format, format_index);

  // Точка означает, что точность была задана, даже если после неё стоит 0.
  if (format[*format_index] == '.') {
    options.precision_set = 1;  // Запоминаем сам факт наличия точки.
    (*format_index)++;          // Пропускаем точку.
    options.precision = s21_parse_number(format, format_index);
  }

  // Реализация поддерживает однобуквенные модификаторы h и l.
  if (format[*format_index] == 'h' || format[*format_index] == 'l') {
    options.length = format[*format_index];
    (*format_index)++;  // Переходим от модификатора к спецификатору.
  }

  // Текущий символ определяет, какое значение потребуется преобразовать.
  options.specifier = format[*format_index];
  if (format[*format_index] != '\0') {
    (*format_index)++;  // Оставляем индекс на первом символе после формата.
  }
  return options;  // Возвращаем полностью заполненные настройки.
}

// Переводит unsigned long в десятичную строку без библиотечного sprintf.
static int s21_unsigned_to_string(unsigned long value, char* destination) {
  char reversed_digits[32];  // Сначала цифры получаются в обратном порядке.
  int digit_count = 0;       // Число уже извлечённых цифр.
  do {
    // Остаток от деления на 10 является последней цифрой числа.
    reversed_digits[digit_count++] = (char)('0' + value % 10);
    value /= 10;  // Удаляем последнюю цифру.
  } while (value > 0);

  // Переворачиваем цифры, чтобы получить привычный порядок слева направо.
  for (int i = 0; i < digit_count; i++) {
    destination[i] = reversed_digits[digit_count - i - 1];
  }
  destination[digit_count] = '\0';  // Завершаем полученную строку.
  return digit_count;               // Возвращаем длину без '\0'.
}

// Выделяет память и записывает начальное значение в большое целое.
static int s21_big_init(s21_big_uint* number, unsigned long long value,
                        int precision) {
  // На один блок приходится 9 цифр; дополнительно оставляем безопасный запас.
  number->capacity = (precision + S21_FLOAT_MARGIN) / 9 + 8;
  number->blocks = calloc((s21_size_t)number->capacity, sizeof(unsigned int));
  number->size = 0;  // Пока ни один блок не заполнен.
  if (number->blocks != S21_NULL) {
    do {
      // Младшие блоки хранятся в начале массива.
      number->blocks[number->size++] = (unsigned int)(value % S21_BIG_BASE);
      value /= S21_BIG_BASE;  // Убираем уже сохранённые 9 цифр.
    } while (value > 0);
  }
  return number->blocks != S21_NULL;  // 1 означает успешное выделение.
}

// Умножает большое целое на небольшой целочисленный множитель.
static void s21_big_multiply(s21_big_uint* number, unsigned int factor) {
  unsigned long long carry = 0;  // Перенос из предыдущего блока.
  for (int i = 0; i < number->size; i++) {
    // 64-битный тип вмещает произведение блока и перенос.
    unsigned long long product =
        (unsigned long long)number->blocks[i] * factor + carry;
    // В блоке оставляем младшие 9 десятичных цифр произведения.
    number->blocks[i] = (unsigned int)(product % S21_BIG_BASE);
    carry = product / S21_BIG_BASE;  // Старшие цифры переносим далее.
  }
  // Если после последнего блока остался перенос, создаём новый блок.
  if (carry > 0 && number->size < number->capacity) {
    number->blocks[number->size++] = (unsigned int)carry;
  }
}

// Делит большое целое на 2 и возвращает отброшенный остаток (0 или 1).
static int s21_big_divide_two(s21_big_uint* number) {
  unsigned long long remainder = 0;  // Остаток старшего блока.
  // Деление выполняется от старших блоков к младшим.
  for (int i = number->size - 1; i >= 0; i--) {
    unsigned long long current = remainder * S21_BIG_BASE + number->blocks[i];
    number->blocks[i] = (unsigned int)(current / 2);
    remainder = current % 2;  // Передаём остаток следующему блоку.
  }
  // Убираем ведущие нулевые блоки, но оставляем хотя бы один.
  while (number->size > 1 && number->blocks[number->size - 1] == 0) {
    number->size--;
  }
  return (int)remainder;  // Остаток нужен для корректного округления.
}

// Прибавляет единицу к большому целому с распространением переноса.
static void s21_big_add_one(s21_big_uint* number) {
  unsigned long long carry = 1;  // Прибавление начинается с единицы.
  int index = 0;                 // Начинаем с младшего блока.
  while (carry > 0 && index < number->size) {
    unsigned long long sum = number->blocks[index] + carry;
    number->blocks[index++] = (unsigned int)(sum % S21_BIG_BASE);
    carry = sum / S21_BIG_BASE;  // Перенос появляется при переполнении блока.
  }
  // Для оставшегося переноса может понадобиться ещё один блок.
  if (carry > 0 && number->size < number->capacity) {
    number->blocks[number->size++] = (unsigned int)carry;
  }
}

// Собирает десятичную строку из блоков большого целого.
static int s21_big_to_string(const s21_big_uint* number, char* destination) {
  int top = number->size - 1;  // Индекс самого старшего блока.
  // Старший блок выводится без ведущих нулей.
  int length = s21_unsigned_to_string(number->blocks[top], destination);
  // Каждый следующий блок обязательно занимает ровно 9 цифр.
  for (int block = top - 1; block >= 0; block--) {
    unsigned int divisor = 100000000;  // Начинаем с разряда 10^8.
    for (int digit = 0; digit < 9; digit++) {
      destination[length++] =
          (char)('0' + number->blocks[block] / divisor % 10);
      divisor /= 10;  // Переходим к следующему десятичному разряду.
    }
  }
  destination[length] = '\0';  // Завершаем строку.
  return length;               // Возвращаем число десятичных цифр.
}
static int s21_scaled_float_to_string(long double value, int precision,
                                      char* destination) {
  int exponent = 0;  // Здесь frexpl сохранит двоичный порядок числа.
  // Представляем value как fraction * 2^exponent.
  long double fraction = frexpl(value, &exponent);
  // Переносим 53 значащих двоичных разряда мантиссы в целое число.
  unsigned long long mantissa = (unsigned long long)ldexpl(fraction, 53);
  // Учитываем снятые 53 бита и требуемое десятичное масштабирование.
  int shift = exponent - 53 + precision;
  s21_big_uint number;  // Здесь будет храниться масштабированная мантисса.
  int length = 0;       // Нулевая длина также сообщает об ошибке выделения.

  // Инициализируем большое целое значением мантиссы.
  if (s21_big_init(&number, mantissa, precision)) {
    // 10^precision = 2^precision * 5^precision; множители 5 добавляем здесь.
    for (int i = 0; i < precision; i++) s21_big_multiply(&number, 5);
    // Положительный shift означает дополнительное умножение на степень 2.
    for (int i = 0; i < shift; i++) s21_big_multiply(&number, 2);
    int lower_bits = 0;  // Были ли единицы ниже разряда половины.
    int half_bit = 0;    // Равна ли отбрасываемая часть как минимум половине.

    // Отрицательный shift означает деление на соответствующую степень 2.
    for (int i = 0; i < -shift; i++) {
      int bit = s21_big_divide_two(&number);  // Запоминаем отброшенный бит.
      if (i == -shift - 1)
        half_bit = bit;  // Последний остаток является разрядом половины.
      else if (bit)
        lower_bits = 1;  // Остальные единицы показывают превышение половины.
    }
    // Округляем к ближайшему; при точной половине — к чётному числу.
    if (half_bit && (lower_bits || number.blocks[0] % 2)) {
      s21_big_add_one(&number);
    }
    length = s21_big_to_string(&number, destination);  // Получаем цифры.
    free(number.blocks);  // Освобождаем блоки большого числа.
  }
  return length;  // Позиция точки пока не учитывается.
}

// Записывает знак числа с приоритетом: минус, плюс, затем пробел.
static int s21_add_integer_sign(char* destination, int negative,
                                const s21_format* options) {
  int length = 0;  // Если знак не нужен, длина останется нулевой.
  if (negative) {
    destination[length++] = '-';  // Отрицательный знак обязателен.
  } else if (options->flag_plus) {
    destination[length++] = '+';  // '+' важнее флага-пробела.
  } else if (options->flag_space) {
    destination[length++] = ' ';  // Пробел резервирует место для знака.
  }
  return length;  // Возвращаем 0 или 1 записанный символ.
}

// Формирует строку для знакового десятичного целого.
static int s21_make_signed(long value, const s21_format* options,
                           char* value_string) {
  int negative = value < 0;  // Запоминаем знак до получения модуля.
  // Беззнаковое вычитание корректно получает модуль даже для LONG_MIN.
  unsigned long magnitude =
      negative ? 0UL - (unsigned long)value : (unsigned long)value;
  char digits[32];  // Временный буфер только для цифр, без знака.
  int digit_count = s21_unsigned_to_string(magnitude, digits);
  // При точности .0 нулевое целое должно быть представлено пустой строкой.
  if (options->precision_set && options->precision == 0 && magnitude == 0) {
    digit_count = 0;
  }
  // Сначала в итоговый буфер помещается знак.
  int result_length = s21_add_integer_sign(value_string, negative, options);
  // Для целого precision задаёт минимальное количество цифр.
  int zero_count = options->precision - digit_count;
  if (zero_count < 0) zero_count = 0;  // Лишние цифры не обрезаются.
  // Недостающие цифры дополняются нулями слева.
  for (int i = 0; i < zero_count; i++) value_string[result_length++] = '0';
  // После знака и нулей копируются цифры исходного числа.
  for (int i = 0; i < digit_count; i++) {
    value_string[result_length++] = digits[i];
  }
  value_string[result_length] = '\0';  // Завершаем временную строку.
  return result_length;                // Возвращаем длину вместе со знаком.
}

// Формирует строку для беззнакового десятичного целого.
static int s21_make_unsigned(unsigned long value, const s21_format* options,
                             char* value_string) {
  char digits[32];  // Временный буфер для десятичных цифр.
  int digit_count = s21_unsigned_to_string(value, digits);
  // Правило %.0u для нуля такое же: ни одной цифры не выводится.
  if (options->precision_set && options->precision == 0 && value == 0) {
    digit_count = 0;
  }
  // Вычисляем, сколько ведущих нулей требует точность.
  int zero_count = options->precision - digit_count;
  if (zero_count < 0) zero_count = 0;  // Уже имеющиеся цифры не удаляются.
  int result_length = 0;               // Текущая позиция во временной строке.
  // Сначала записываем ведущие нули.
  for (int i = 0; i < zero_count; i++) value_string[result_length++] = '0';
  // Затем записываем само число.
  for (int i = 0; i < digit_count; i++) {
    value_string[result_length++] = digits[i];
  }
  value_string[result_length] = '\0';  // Добавляем конец C-строки.
  return result_length;                // Возвращаем число символов.
}

// Формирует %f, включая знак, конечные значения, infinity и NaN.
static int s21_make_float(long double value, const s21_format* options,
                          char* value_string) {
  int result_length = 0;           // Позиция записи во временном буфере.
  int negative = signbit(value) != 0;  // signbit различает также -0.0.
  if (negative) value = -value;    // Дальше работаем с модулем числа.

  // NaN форматируется отдельно, поскольку у него нет обычных цифр.
  if (isnan(value)) {
    // libc на macOS не выводит знак NaN, а Linux может его учитывать.
#if !defined(__APPLE__) && !defined(__MACH__)
    result_length = s21_add_integer_sign(value_string, negative, options);
#endif
    value_string[result_length++] = 'n';  // Последовательно записываем "nan".
    value_string[result_length++] = 'a';
    value_string[result_length++] = 'n';
  } else {
    // Для infinity и конечных чисел действуют обычные правила знака.
    result_length = s21_add_integer_sign(value_string, negative, options);
    if (isinf(value)) {
      value_string[result_length++] = 'i';  // Формируем текст "inf".
      value_string[result_length++] = 'n';
      value_string[result_length++] = 'f';
    } else {
      // Стандартная точность %f при отсутствии точки в формате равна 6.
      int precision = options->precision_set ? options->precision : 6;
      // Числовая часть пишется сразу после уже добавленного знака.
      result_length +=
          s21_make_finite_float(value, precision, value_string + result_length);
    }
  }
  value_string[result_length] = '\0';  // Завершаем представление числа.
  return result_length;                // Возвращаем длину со знаком.
}

// Превращает конечное положительное число в запись с фиксированной точкой.
static int s21_make_finite_float(long double value, int precision,
                                 char* value_string) {
  // scaled хранит округлённое value * 10^precision без десятичной точки.
  char* scaled = malloc((s21_size_t)precision + S21_FLOAT_MARGIN);
  int scaled_length = 0;  // Количество цифр в масштабированном числе.
  int result_length = 0;  // Позиция записи в value_string.
  if (scaled != S21_NULL) {
    // Получаем строку цифр уже с правильным округлением.
    scaled_length = s21_scaled_float_to_string(value, precision, scaled);
    // Если целая часть равна нулю, явно добавляем "0." и ведущие нули дроби.
    if (precision > 0 && scaled_length <= precision) {
      value_string[result_length++] = '0';  // Ноль перед точкой.
      value_string[result_length++] = '.';  // Десятичный разделитель.
      for (int i = scaled_length; i < precision; i++) {
        value_string[result_length++] = '0';  // Нули до значащих цифр.
      }
    }
    // Копируем цифры и вставляем точку перед последними precision цифрами.
    for (int i = 0; i < scaled_length; i++) {
      if (precision > 0 && scaled_length > precision &&
          i == scaled_length - precision) {
        value_string[result_length++] = '.';
      }
      value_string[result_length++] = scaled[i];
    }
    free(scaled);  // Временная строка больше не нужна.
  }
  value_string[result_length] = '\0';  // Завершаем готовое значение %f.
  return result_length;                // Возвращаем длину с точкой.
}
static void s21_write_field(char* str, int* result_length, const char* value,
                            int value_length, const s21_format* options) {
  // Разница между шириной поля и длиной значения заполняется пробелами.
  int padding = options->width - value_length;
  if (padding < 0) padding = 0;  // Слишком длинное значение не обрезается.

  // По умолчанию значение выравнивается вправо: пробелы идут перед ним.
  if (!options->flag_minus) {
    for (int i = 0; i < padding; i++) str[(*result_length)++] = ' ';
  }

  // Копируем подготовленное значение в общий выходной буфер.
  for (int i = 0; i < value_length; i++) {
    str[(*result_length)++] = value[i];
  }

  // Флаг '-' включает выравнивание влево: пробелы идут после значения.
  if (options->flag_minus) {
    for (int i = 0; i < padding; i++) str[(*result_length)++] = ' ';
  }
}

// Выбирает обработчик по спецификатору и записывает готовое поле в str.
static void s21_process_format(char* str, int* result_length,
                               const s21_format* options, va_list* arguments) {
  // Строку можно писать непосредственно, не создавая временную копию.
  if (options->specifier == 's') {
    // Извлекаем следующий аргумент как указатель на строку.
    const char* value = va_arg(*arguments, const char*);
    // Точность для %s ограничивает максимальное число символов.
    int length = s21_string_length(value, options);
    // Учитываем ширину поля и возможное выравнивание влево.
    s21_write_field(str, result_length, value, length, options);
  } else {
    // Для остальных типов сначала создаётся их строковое представление.
    int capacity = s21_value_capacity(options);
    char* value_string = malloc((s21_size_t)capacity);
    int value_length = 0;  // Число символов во временном представлении.

    // Продолжаем обработку только при успешном выделении памяти.
    if (value_string != S21_NULL) {
      if (options->specifier == '%') {
        value_string[value_length++] = '%';  // %% не читает аргумент.
      } else if (options->specifier == 'c') {
        // char в вариативной функции передаётся как int из-за promotion.
        value_string[value_length++] = (char)va_arg(*arguments, int);
      } else if (options->specifier == 'd') {
        // Читаем int/short/long в зависимости от модификатора длины.
        long value = s21_read_signed(options, arguments);
        // Добавляем знак, точность и десятичные цифры.
        value_length = s21_make_signed(value, options, value_string);
      } else if (options->specifier == 'u') {
        // Читаем соответствующий беззнаковый целочисленный тип.
        unsigned long value = s21_read_unsigned(options, arguments);
        // Добавляем ведущие нули точности и цифры.
        value_length = s21_make_unsigned(value, options, value_string);
      } else if (options->specifier == 'f') {
        // float в ... повышается до double; затем расширяем его до long double.
        long double value = (long double)va_arg(*arguments, double);
        // Создаём запись с фиксированной точкой.
        value_length = s21_make_float(value, options, value_string);
      }

      value_string[value_length] = '\0';  // Страхуем конец временной строки.
      // Переносим значение в str, добавляя пробелы до требуемой ширины.
      s21_write_field(str, result_length, value_string, value_length, options);
      free(value_string);  // Освобождаем временный буфер.
    }
  }
}

// Извлекает знаковое целое с учётом модификатора h или l.
static long s21_read_signed(const s21_format* options, va_list* arguments) {
  long value;  // Общий тип, в который помещаются все поддерживаемые варианты.
  if (options->length == 'l') {
    value = va_arg(*arguments, long);  // %ld получает long без приведения.
  } else if (options->length == 'h') {
    // short передаётся через ... как int, поэтому сначала читается int.
    value = (short)va_arg(*arguments, int);
  } else {
    value = va_arg(*arguments, int);  // Обычный %d получает int.
  }
  return value;  // Возвращаем всё как long для единого форматирования.
}

// Извлекает беззнаковое целое с учётом модификатора h или l.
static unsigned long s21_read_unsigned(const s21_format* options,
                                       va_list* arguments) {
  unsigned long value;  // Общий тип для всех поддерживаемых вариантов.
  if (options->length == 'l') {
    // %lu получает аргумент типа unsigned long.
    value = va_arg(*arguments, unsigned long);
  } else if (options->length == 'h') {
    // unsigned short при передаче повышается до int/unsigned int.
    value = (unsigned short)va_arg(*arguments, unsigned int);
  } else {
    // Обычный %u получает unsigned int.
    value = va_arg(*arguments, unsigned int);
  }
  return value;  // Возвращаем расширенное unsigned long.
}

// Определяет, сколько символов строки разрешено вывести для %s.
static int s21_string_length(const char* value, const s21_format* options) {
  int length = 0;  // Одновременно длина и индекс текущего символа.
  // Без precision место не ограничено; с precision нельзя превысить лимит.
  int has_room = !options->precision_set || length < options->precision;
  while (value[length] != '\0' && has_room) {
    length++;  // Учитываем очередной разрешённый символ.
    // Пересчитываем условие перед следующей итерацией.
    has_room = !options->precision_set || length < options->precision;
  }
  return length;  // Это длина, которую затем скопирует s21_write_field.
}

// Рассчитывает размер временного буфера для одного форматируемого значения.
static int s21_value_capacity(const s21_format* options) {
  // 34 символов достаточно для long, знака, символа и завершающего '\0'.
  int capacity = 34;
  if (options->specifier == 'f') {
    // Для %f память зависит от заданного числа знаков после точки.
    int precision = options->precision_set ? options->precision : 6;
    // Запас покрывает целую часть, знак, точку и внутренние вычисления.
    capacity = precision + S21_FLOAT_MARGIN;
  } else if ((options->specifier == 'd' || options->specifier == 'u') &&
             options->precision + 3 > capacity) {
    // Большая точность целого требует места для нулей, знака и '\0'.
    capacity = options->precision + 3;
  }
  return capacity;  // Этот размер передаётся в malloc.
}
