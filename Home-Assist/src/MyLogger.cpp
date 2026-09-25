#include "MyLogger.h"
#include <stdarg.h>

MyLogger gLogger;

void MyLogger::add(const char* line) {
    char* dest  = _lines[_head];
    int   offset = 0;

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

    if (!_publishing && _publishFn) {
        _publishing = true;
        _publishFn(dest);
        _publishing = false;
    }

    _head = (_head + 1) % LOG_MAX_LINES;
    if (_count < LOG_MAX_LINES) _count++;
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

String MyLogger::getContent() const {
    if (_count == 0) return "(aucun log)\n";
    String result;
    uint8_t start = (_count < LOG_MAX_LINES) ? 0 : _head;
    for (uint8_t i = 0; i < _count; i++) {
        result += _lines[(start + i) % LOG_MAX_LINES];
        result += '\n';
    }
    return result;
}

void MyLogger::clear() {
    _head  = 0;
    _count = 0;
}
