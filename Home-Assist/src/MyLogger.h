#pragma once
#include <Arduino.h>

//------------------------------------------------------------------------------
// Publication des traces DBG / DBGLN (global.h) sur MQTT home/log, horodatées.
// Le journal de la page web (/logs) est tenu par MyJournal (tout ce qui passe par Serial) :
// MyLogger ne garde plus de lignes en mémoire.
//------------------------------------------------------------------------------
constexpr uint8_t LOG_LINE_LEN  = 200;

typedef void   (*LogPublishFn)(const char*);
typedef String (*LogTimeFn)();

class MyLogger {
public:
    void add(const char* line);
    void addf(const char* format, ...);
    void setPublishCallback(LogPublishFn fn);
    void setTimeCallback(LogTimeFn fn);
private:
    LogPublishFn _publishFn  = nullptr;
    LogTimeFn    _timeFn     = nullptr;
    bool         _publishing = false;
};

extern MyLogger gLogger;
