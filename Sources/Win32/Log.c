
#include <stdio.h>
#include <stdarg.h>

#include "Log.h"

void LogPrint(const char * format, ...)
{
	va_list args;
    va_start(args, format);

	vprintf(format, args);

    va_end(args);
}
