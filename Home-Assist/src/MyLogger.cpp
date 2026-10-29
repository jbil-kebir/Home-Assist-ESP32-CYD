#include "MyLogger.h"
#include <stdarg.h>

MyLogger gLogger;

void MyLogger::add(const char* line) {
    if (_publishing || !_publishFn) return;

    char dest[LOG_LINE_LEN];
    int  offset = 0;

    if (_timeFn) {
        String ts = "[" + _timeFn() + "] ";
        int tsLen = ts.length();
        if (tsLen > LOG_LINE_LEN - 2) tsLen = LOG_LINE_LEN - 2;
        memcpy(dest, ts.c_str(), tsLen);
        offset = tsLen;
    }

    strncpy(dest + offset, line, LOG_LINE_LEN - 1 - offset);
    dest[LOG_LINE_LEN - 1] = '\0';

    int len = strlen(dest);
    while (len > 0 && (dest[len - 1] == '\n' || dest[len - 1] == '\r'))
        dest[--len] = '\0';
    if (len == 0) return;

    _publishing = true; // Une trace émise pendant la publication n'est pas republiée
    _publishFn(dest);
    _publishing = false;
}

void MyLogger::setPublishCallback(LogPublishFn fn) {
    _publishFn = fn;
}

void MyLogger::setTimeCallback(LogTimeFn fn) {
    _timeFn = fn;
}

void MyLogger::addf(const char* format, ...) {
    char buf[LOG_LINE_LEN];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    add(buf);
}
