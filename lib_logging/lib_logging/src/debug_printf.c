// Copyright 2014-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <debug_print.h>
#include <print.h>
#include <stdarg.h>
#include <syscall.h>
#include <limits.h>
#include <print.h>
#include <string.h>
#include <ctype.h>

#undef debug_printf

static void reverse_array(char buf[], unsigned size)
{
  int begin = 0;
  int end = size - 1;
  int tmp;
  for (;begin < end; begin++,end--) {
    tmp = buf[begin];
    buf[begin] = buf[end];
    buf[end] = tmp;
  }
}

static int itoa(unsigned n, char *buf, unsigned base, int fill)
{
  static const char digits[] = "0123456789ABCDEF";
  unsigned i = 0;

  if (n == 0)
    fill += 1;

  while (n > 0) {
    unsigned next = n / base;
    unsigned cur  = n % base;
    buf[i] = digits[cur];
    i += 1;
    fill--;
    n = next;
  }
  for (;fill > 0; fill--) {
    buf[i] = '0';
    i++;
  }
  reverse_array(buf, i);
  return i;
}

static int ftoa(double val, char *buf)
{
  int integer = (int)val;
  int precision = (1000000000);   // for highest precision
  int cal_zero;
  unsigned idx = 0;

  val = val - integer;
  idx += itoa(integer, buf, 10, 0);
  buf[idx] = '.';
  idx += 1;

  val = val * precision;
  val += 500;    // round the value
  val /= 1000;   // only show 6 digts, which is same as default %f
  //caculate decimal part zero
  cal_zero = precision / 1000 / val;
  for (int i = 0; cal_zero > 0 && i < 6;  i++) {
    cal_zero /= 10;
    if (cal_zero > 0) {
      buf[idx] = '0';
      idx += 1;
    } else {
      break;
    }
  }
  if(val >= 1.0){
    idx += itoa((int)val, buf + idx, 10, 0);
  }

  return idx;
}

#define MAX_INT_STRING_SIZE 10

#ifndef DEBUG_PRINTF_BUFSIZE
#define DEBUG_PRINTF_BUFSIZE 130
#endif


void debug_printf(char * fmt, ...)
{
  char * marker;
  int intArg;
  unsigned int uintArg;
  double doubleArg;
  char * strArg;

  char buf[DEBUG_PRINTF_BUFSIZE];
  char *end = &buf[DEBUG_PRINTF_BUFSIZE - 1 - MAX_INT_STRING_SIZE];

  va_list args;

  va_start(args,fmt);
  marker = fmt;
  char *p = buf;
  while (*fmt) {
    if (p > end) {
      // flush
      _write(FD_STDOUT, buf, p - buf);
      p = buf;
    }
    switch (*fmt) {
    case '%':
      fmt++;
      if (*(fmt) == '-' || *(fmt) == '+' || *(fmt) == '#' || *(fmt) == ' ' || *(fmt) == 'l') {
        // Ignore flags
        fmt++;
      }
      while (*(fmt) && *(fmt) >= '0' && *(fmt) <= '9') {
        // Ignore width
        fmt++;
      }
      // Use 'tolower' to ensure both %x/%X do something sensible
      switch (tolower(*(fmt))) {
      case 'd':
        intArg = va_arg(args, int);
        if (intArg < 0) {
          *p++ = '-';
          intArg = -intArg;
        }
        p += itoa(intArg, p, 10, 0);
        break;
      case 'u':
        uintArg = va_arg(args, int);
        p += itoa(uintArg, p, 10, 0);
        break;
      case 'p':
      case 'x':
        uintArg = va_arg(args, int);
        p += itoa(uintArg, p, 16, 0);
        break;
      case 'f':
        doubleArg=va_arg(args,double);
        if (doubleArg<0) {
          *p++ = '-';
          doubleArg=-doubleArg;
        }
        p +=ftoa(doubleArg, p);
        break;
      case 'c':
        intArg = va_arg(args, int);
        *p++ = intArg;
        break;
      case 's':
        strArg = va_arg(args, char *);
        int len = strlen(strArg);
        if (len > (end - buf)) {
                // flush
          _write(FD_STDOUT, buf, p - buf);
          p = buf;
        }
        if (len > (end - buf))
          len = end - buf;
        memcpy(p, strArg, len);
        p += len;
        break;
      default:
        break;
      }
      break;

    default:
      *p++ = *fmt;
    }
    fmt++;
  }
  _write(FD_STDOUT, buf, p - buf);
  va_end(args);

  return;
}


