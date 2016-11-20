
#include <jni.h>
#include <errno.h>

#include <stdio.h>
#include <stdarg.h>
#include <android/log.h>

#include "Log.h"

void LogPrint(const char * format, ...)
{
	va_list args;
    va_start(args, format);

    __android_log_vprint(ANDROID_LOG_INFO, "native-activity", format, args);

    va_end(args);
}
