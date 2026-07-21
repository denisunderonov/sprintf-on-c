#include "s21_string.h"

#if defined(__APPLE__) || defined(__MACH__)
#define UNKNOWN_ERR "Unknown error: "
#define ERR_COUNT 108
static const char* ERRORS[ERR_COUNT] = {
    "Undefined error: 0",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "Input/output error",
    "Device not configured",
    "Argument list too long",
    "Exec format error",
    "Bad file descriptor",
    "No child processes",
    "Resource deadlock avoided",
    "Cannot allocate memory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Resource busy",
    "File exists",
    "Cross-device link",
    "Operation not supported by device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "Too many open files in system",
    "Too many open files",
    "Inappropriate ioctl for device",
    "Text file busy",
    "File too large",
    "No space left on device",
    "Illegal seek",
    "Read-only file system",
    "Too many links",
    "Broken pipe",
    "Numerical argument out of domain",
    "Result too large",
    "Resource temporarily unavailable",
    "Operation now in progress",
    "Operation already in progress",
    "Socket operation on non-socket",
    "Destination address required",
    "Message too long",
    "Protocol wrong type for socket",
    "Protocol not available",
    "Protocol not supported",
    "Socket type not supported",
    "Operation not supported",
    "Protocol family not supported",
    "Address family not supported by protocol family",
    "Address already in use",
    "Can't assign requested address",
    "Network is down",
    "Network is unreachable",
    "Network dropped connection on reset",
    "Software caused connection abort",
    "Connection reset by peer",
    "No buffer space available",
    "Socket is already connected",
    "Socket is not connected",
    "Can't send after socket shutdown",
    "Too many references: can't splice",
    "Operation timed out",
    "Connection refused",
    "Too many levels of symbolic links",
    "File name too long",
    "Host is down",
    "No route to host",
    "Directory not empty",
    "Too many processes",
    "Too many users",
    "Disc quota exceeded",
    "Stale NFS file handle",
    "Too many levels of remote in path",
    "RPC struct is bad",
    "RPC version wrong",
    "RPC prog. not avail",
    "Program version wrong",
    "Bad procedure for program",
    "No locks available",
    "Function not implemented",
    "Inappropriate file type or format",
    "Authentication error",
    "Need authenticator",
    "Device power is off",
    "Device error",
    "Value too large to be stored in data type",
    "Bad executable (or shared library)",
    "Bad CPU type in executable",
    "Shared library version mismatch",
    "Malformed Mach-o file",
    "Operation canceled",
    "Identifier removed",
    "No message of desired type",
    "Illegal byte sequence",
    "Attribute not found",
    "Bad message",
    "EMULTIHOP (Reserved)",
    "No message available on STREAM",
    "ENOLINK (Reserved)",
    "No STREAM resources",
    "Not a STREAM",
    "Protocol error",
    "STREAM ioctl timeout",
    "Operation not supported on socket",
    "Policy not found",
    "State not recoverable",
    "Previous owner died",
    "Interface output queue is full",
    "Capabilities insufficient"};
#endif

#if defined(__linux__)
#define UNKNOWN_ERR "Unknown error "
#define ERR_COUNT 134
static const char* ERRORS[ERR_COUNT] = {
    "Success",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "Input/output error",
    "No such device or address",
    "Argument list too long",
    "Exec format error",
    "Bad file descriptor",
    "No child processes",
    "Resource temporarily unavailable",
    "Cannot allocate memory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Device or resource busy",
    "File exists",
    "Invalid cross-device link",
    "No such device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "Too many open files in system",
    "Too many open files",
    "Inappropriate ioctl for device",
    "Text file busy",
    "File too large",
    "No space left on device",
    "Illegal seek",
    "Read-only file system",
    "Too many links",
    "Broken pipe",
    "Numerical argument out of domain",
    "Numerical result out of range",
    "Resource deadlock avoided",
    "File name too long",
    "No locks available",
    "Function not implemented",
    "Directory not empty",
    "Too many levels of symbolic links",
    "Unknown error 41",
    "No message of desired type",
    "Identifier removed",
    "Channel number out of range",
    "Level 2 not synchronized",
    "Level 3 halted",
    "Level 3 reset",
    "Link number out of range",
    "Protocol driver not attached",
    "No CSI structure available",
    "Level 2 halted",
    "Invalid exchange",
    "Invalid request descriptor",
    "Exchange full",
    "No anode",
    "Invalid request code",
    "Invalid slot",
    "Unknown error 58",
    "Bad font file format",
    "Device not a stream",
    "No data available",
    "Timer expired",
    "Out of streams resources",
    "Machine is not on the network",
    "Package not installed",
    "Object is remote",
    "Link has been severed",
    "Advertise error",
    "Srmount error",
    "Communication error on send",
    "Protocol error",
    "Multihop attempted",
    "RFS specific error",
    "Bad message",
    "Value too large for defined data type",
    "Name not unique on network",
    "File descriptor in bad state",
    "Remote address changed",
    "Can not access a needed shared library",
    "Accessing a corrupted shared library",
    ".lib section in a.out corrupted",
    "Attempting to link in too many shared libraries",
    "Cannot exec a shared library directly",
    "Invalid or incomplete multibyte or wide character",
    "Interrupted system call should be restarted",
    "Streams pipe error",
    "Too many users",
    "Socket operation on non-socket",
    "Destination address required",
    "Message too long",
    "Protocol wrong type for socket",
    "Protocol not available",
    "Protocol not supported",
    "Socket type not supported",
    "Operation not supported",
    "Protocol family not supported",
    "Address family not supported by protocol",
    "Address already in use",
    "Cannot assign requested address",
    "Network is down",
    "Network is unreachable",
    "Network dropped connection on reset",
    "Software caused connection abort",
    "Connection reset by peer",
    "No buffer space available",
    "Transport endpoint is already connected",
    "Transport endpoint is not connected",
    "Cannot send after transport endpoint shutdown",
    "Too many references: cannot splice",
    "Connection timed out",
    "Connection refused",
    "Host is down",
    "No route to host",
    "Operation already in progress",
    "Operation now in progress",
    "Stale file handle",
    "Structure needs cleaning",
    "Not a XENIX named type file",
    "No XENIX semaphores available",
    "Is a named type file",
    "Remote I/O error",
    "Disk quota exceeded",
    "No medium found",
    "Wrong medium type",
    "Operation canceled",
    "Required key not available",
    "Key has expired",
    "Key has been revoked",
    "Key was rejected by service",
    "Owner died",
    "State not recoverable",
    "Operation not possible due to RF-kill",
    "Memory page has hardware error"};
#endif

char* s21_strncat(char* dest, const char* src, s21_size_t n) {
  s21_size_t i = 0;
  s21_size_t len_dest = s21_strlen(dest);

  for (; i < n && src[i]; i++) {
    dest[len_dest + i] = src[i];
  }

  dest[len_dest + i] = '\0';

  return dest;
}

char* s21_strchr(const char* str, int c) {
  int found = 0;
  char* result = S21_NULL;
  s21_size_t len = s21_strlen(str);

  for (s21_size_t i = 0; i <= len && !found; i++) {
    if (str[i] == (char)c) {
      found = 1;
      result = (char*)&str[i];
    }
  }

  return result;
}

int s21_strncmp(const char* str1, const char* str2, s21_size_t n) {
  int success = 1;
  int result = 0;

  for (s21_size_t i = 0; i < n && success; i++) {
    if (str1[i] != str2[i]) {
      success = 0;
      result = (unsigned char)(str1[i]) - (unsigned char)(str2[i]);
    } else if (str1[i] == '\0') {
      success = 0;
    }
  }

  return result;
}

char* s21_strncpy(char* dest, const char* src, s21_size_t n) {
  s21_size_t i = 0;

  for (; i < n && src[i]; i++) {
    dest[i] = src[i];
  }

  for (; i < n; i++) {
    dest[i] = '\0';
  }

  return dest;
}

s21_size_t s21_strcspn(const char* str1, const char* str2) {
  int success = 1;
  s21_size_t result = s21_strlen(str1);

  for (s21_size_t i = 0; str1[i] && success; i++) {
    for (s21_size_t j = 0; str2[j] && success; j++) {
      if (str1[i] == str2[j]) {
        success = 0;
        result = i;
      }
    }
  }

  return result;
}

static void s21_make_unknown_error(char* destination, int errnum) {
  int result_length = 0;
  int prefix_index = 0;
  while (UNKNOWN_ERR[prefix_index] != '\0') {
    destination[result_length++] = UNKNOWN_ERR[prefix_index++];
  }
  int negative = errnum < 0;
  if (negative) destination[result_length++] = '-';
  unsigned int magnitude =
      negative ? 0U - (unsigned int)errnum : (unsigned int)errnum;
  char reversed_digits[16];
  int digit_count = 0;
  do {
    reversed_digits[digit_count++] = (char)('0' + magnitude % 10);
    magnitude /= 10;
  } while (magnitude > 0);
  for (int i = digit_count - 1; i >= 0; i--) {
    destination[result_length++] = reversed_digits[i];
  }
  destination[result_length] = '\0';
}

char* s21_strerror(int errnum) {
  static char error_buf[300];
  char* result = S21_NULL;

  if (errnum < ERR_COUNT && errnum >= 0) {
    result = (char*)ERRORS[errnum];
  } else {
    s21_make_unknown_error(error_buf, errnum);
    result = error_buf;
  }

  return result;
}

s21_size_t s21_strlen(const char* str) {
  s21_size_t len = 0;

  for (; str[len]; len++);

  return len;
}

char* s21_strpbrk(const char* str1, const char* str2) {
  char* result = S21_NULL;
  s21_size_t idx = s21_strcspn(str1, str2);

  if (str1[idx] != '\0') {
    result = (char*)&str1[idx];
  }

  return result;
}

char* s21_strrchr(const char* str, int c) {
  char* result = S21_NULL;
  s21_size_t len = s21_strlen(str);

  for (s21_size_t i = 0; i <= len; i++) {
    if (str[i] == (char)c) {
      result = (char*)&str[i];
    }
  }

  return result;
}

char* s21_strstr(const char* haystack, const char* needle) {
  int found = 0;
  char* result = S21_NULL;

  if (!*needle) {
    result = (char*)haystack;
  }

  while (*haystack && !found) {
    const char* h = haystack;
    const char* n = needle;

    while (*n && *n == *h) {
      n++;
      h++;
    }

    if (*n == '\0') {
      found = 1;
      result = (char*)haystack;
    } else {
      haystack++;
    }
  }

  return result;
}

static int s21_is_delim(const char* str, const char* delim) {
  int is_delim = 0;

  for (int i = 0; delim[i] && !is_delim; i++) {
    if (*str == delim[i]) {
      is_delim = 1;
    }
  }

  return is_delim;
}

char* s21_strtok(char* str, const char* delim) {
  static char* next = S21_NULL;
  char* result = S21_NULL;

  if (str != S21_NULL) {
    next = str;
  }

  while (*next && s21_is_delim(next, delim)) {
    next++;
  }

  char* start_token = next;
  int is_delim = 0;

  while (!is_delim && *next) {
    is_delim = s21_is_delim(next, delim);

    if (is_delim) {
      *next = '\0';
      result = start_token;
    }
    next++;
  }

  if (!*next && !is_delim && *start_token) {
    result = start_token;
  }

  return result;
}
