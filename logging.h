#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED
    #include <stdio.h>
    #include <stdlib.h>
    #include <time.h>
    #include <string.h>
    #include <stdarg.h>

    #include "Console_utils.h"
    #include "asserts.h"
    #include "time_utils.h"
    #include "debug_utils.h"

    extern FILE* _logfile;


    void printf_log(const char* format, ...);

    void perror_log_(const char* errmsg, const char* add_info);

    void warn_log(const char* format, ...);

    void info_log(const char* format, ...);

    void debug_log(const char* format, ...);


    #define perror_log(errmsg)      \
    setConsoleColor(stderr, (consoleColor)(COLOR_RED | COLOR_INTENSE), COLOR_BLACK); \
    fprint_time_nodate(_logfile, time(nullptr));                                     \
    fprintf(_logfile, "[ERROR] %s :%s\n at: \nFile:%s \nLine:%d \nFunc:%s\n",        \
              errmsg, strerror(errno), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
    fprintf(stderr  , "[ERROR] %s :%s\n at: \nFile:%s \nLine:%d \nFunc:%s\n",        \
              errmsg, strerror(errno), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE | COLOR_INTENSE), COLOR_BLACK);


    #define error_log(format, ...)                                                     \
    setConsoleColor(stderr, (consoleColor)(COLOR_RED | COLOR_INTENSE), COLOR_BLACK);   \
    fprint_time_nodate(_logfile, time(nullptr));                                       \
    fprintf(_logfile, "[ERROR]" format , __VA_ARGS__);                                 \
    fprintf(stderr  , "[ERROR]" format , __VA_ARGS__);                                 \
    fprintf(_logfile, " at: \nFile:%s \nLine:%d \nFunc:%s\n",                          \
               __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
    fprintf(stderr  , " at: \nFile:%s \nLine:%d \nFunc:%s\n",                          \
               __FILE__, __LINE__, __PRETTY_FUNCTION__);       \
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE | COLOR_INTENSE), COLOR_BLACK);


    #define $log(operation)                                  \
    debug_log(#operation " at " __FILE__ ": %d\n", __LINE__);  \
    operation;


    void printVarInfo_log(const VarInfo *var);

#endif // LOGGING_H_INCLUDED
