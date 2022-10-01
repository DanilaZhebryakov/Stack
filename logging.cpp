#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

#include "time_utils.h"
#include "Console_utils.h"
#include "debug_utils.h"

static const int max_print_args = 100;

FILE* initLogFile();

FILE* _logfile = initLogFile();

void printGoodbyeMsg(){
    fprintf(_logfile, "Program exited\n");
}

FILE* initLogFile(){
    FILE* logfile = fopen("log.log", "a");
    if (logfile == nullptr){
        perror("Error opening log file");
    }

    if (setvbuf(logfile, nullptr, _IONBF, 0) != 0){
        perror("Warning: can not set log file buffer");
    }

    fprintf(logfile, "------------------------------------\n");
    fprint_time_date_short(logfile, time(nullptr));
    fprintf(logfile, "Program started\n");
    atexit(printGoodbyeMsg);

    return logfile;
}

void printVarInfo_log(const VarInfo *var){
    setConsoleColor(stderr, COLOR_WHITE, COLOR_BLACK);

    if(var != nullptr){
    fprintf(_logfile, "     Variable info:     Name: %s\n"
                      "                      Status: %s\n"
                      "                  Created at: %s :%d\n"
                      "                 In function: %s\n",
    strPrintable(var->name), varstatusAsString(var->status) , strPrintable(var->file), var->line, strPrintable(var->func));

    fprintf( stderr , "     Variable info:     Name: %s\n"
                      "                      Status: %s\n"
                      "                  Created at: %s :%d\n"
                      "                 In function: %s\n",
    strPrintable(var->name), varstatusAsString(var->status) , strPrintable(var->file), var->line, strPrintable(var->func));
    }
    else{
        fprintf(_logfile, "     Variable info: info is nullptr\n");
        fprintf( stderr , "     Variable info: info is nullptr\n");
    }

    setConsoleColor(stderr, COLOR_DEFAULTT, COLOR_BLACK);

}

void printf_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setConsoleColor(stderr, COLOR_WHITE, COLOR_BLACK);
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE | COLOR_INTENSE), COLOR_BLACK);
    va_end(args);
}

void warn_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setConsoleColor(stderr, (consoleColor)(COLOR_YELLOW | COLOR_INTENSE), COLOR_BLACK);
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[WARN]");
    fprintf( stderr , "[WARN]");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE | COLOR_INTENSE), COLOR_BLACK);
    va_end(args);
}
void info_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE), COLOR_BLACK);
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[info]");
    fprintf( stderr , "[info]");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE | COLOR_INTENSE), COLOR_BLACK);
    va_end(args);
}

void debug_log(const char* format, ...){
    va_list args;
    va_start(args, max_print_args);
    setConsoleColor(stderr, COLOR_MAGENTA, COLOR_BLACK);
    fprint_time_nodate(_logfile, time(nullptr));
    fprintf(_logfile, "[DEBUG]");
    fprintf( stderr , "[DEBUG]");
    vfprintf(_logfile, format , args);
    vfprintf( stderr , format , args);
    setConsoleColor(stderr, (consoleColor)(COLOR_WHITE | COLOR_INTENSE), COLOR_BLACK);
    va_end(args);
}
void dumpData(const void* begin_ptr, size_t max_size){
    printf_log("     Raw data dump: (%ld total) [ ", max_size);
    size_t i = 0;
    while (i < max_size){
        if(!IsBadReadPtr((char*)begin_ptr + i, 1)){
            printf_log("%p ", ((uint8_t*)begin_ptr)[i]);
        }
        else{
            printf_log("|access denied| ");
        }
        i++;
    }
    printf_log("]\n");
}
