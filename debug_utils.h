#ifndef DEBUG_UTILS_H_INCLUDED
#define DEBUG_UTILS_H_INCLUDED

#include <stdint.h>

enum variableStatus_t{
    VARSTATUS_NEW    = 0,
    VARSTATUS_UNUSED = 1,
    VARSTATUS_NORMAL = 2,
    VARSTATUS_DEAD   = 666
};

const char* varstatusAsString(variableStatus_t var);

struct VarInfo{
    const char* name;
    variableStatus_t status = VARSTATUS_NEW;
    const char* file;
    int line;
    const char* func;
};

const char* strPrintable(const char* str);

#ifdef varInfoInit
    #error redefinition of internal macro varInfoInit()
#endif
#define varInfoInit(var) {#var, VARSTATUS_UNUSED, __FILE__, __LINE__, __PRETTY_FUNCTION__}

//canary protection
typedef uint64_t canary_t;
const canary_t CANARY_L = 0xDEADBEEFDEADBEEF;
const canary_t CANARY_R = 0xFACEFEEDFACEFEED;

bool checkLCanary(const void* ptr);
bool checkRCanary(const void* ptr, size_t len);


//hash function
typedef uint64_t hash_t;
const hash_t HASH_DEFAULT = 0xDEFEC8EDBAADBEEF;

hash_t gnuHash(const void* begin_ptr, const void* end_ptr);

#endif // DEBUG_UTILS_H_INCLUDED
