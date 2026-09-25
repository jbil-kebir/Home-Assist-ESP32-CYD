#pragma once
#include <Arduino.h>

constexpr uint8_t LOG_MAX_LINES = 50;
constexpr uint8_t LOG_LINE_LEN  = 200;

typedef void   (*LogPublishFn)(const char*);
typedef String (*LogTimeFn)();

class MyLogger {
public:
    void add(const char* line);
    void addf(const char* format, ...);
    String getContent() const;
    void clear();
    void setPublishCallback(LogPublishFn fn);
    void setTimeCallback(LogTimeFn fn);
private:
    char         _lines[LOG_MAX_LINES][LOG_LINE_LEN];
    uint8_t      _head       = 0;
    uint8_t      _count      = 0;
    LogPublishFn _publishFn  = nullptr;
    LogTimeFn    _timeFn     = nullptr;
    bool         _publishing = false;
};

extern MyLogger gLogger;
