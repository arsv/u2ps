#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "warn.h"

void warn(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
};

void die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(-1);
};
