#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "asserts.h"
#include "logging.h"
#include "debug_utils.h"


enum stackError_t{
    STACK_NOERROR           = 0,

    STACK_NULL              = 1 << 0,
    STACK_BAD               = 1 << 1,
    STACK_DEAD              = 1 << 2,
    STACK_DATA_NULL         = 1 << 3,
    STACK_DATA_BAD          = 1 << 4,  //NYI
    STACK_SIZE_CAP_BAD      = 1 << 5,

    STACK_CANARY_L_BAD      = 1 << 8,
    STACK_CANARY_R_BAD      = 1 << 9,
    STACK_DATA_CANARY_L_BAD = 1 << 10,
    STACK_DATA_CANARY_R_BAD = 1 << 11,

    STACK_HASH_BAD          = 1 << 16,
    STACK_DATA_HASH_BAD     = 1 << 17,

    STACK_OP_INVALID        = 1 << 24,
    STACK_OP_ERROR          = 1 << 25
};

//#define NO_UNSAFE_PRINT

#ifndef ELEM_T
    #define ELEM_T int
#endif

#ifndef ELEM_SPEC
    #define ELEM_SPEC "%d"
#endif

#ifndef BAD_ELEM
    #define BAD_ELEM 133
#endif

#ifndef DESTRUCT_PTR
    #define DESTRUCT_PTR ((ELEM_T*)0xBAD)
#endif

#ifndef STACK_MIN_SIZE
    #define STACK_MIN_SIZE 10
#endif


#define stackCheckRet(__stk, ...)  \
if(stackError(__stk)){             \
    error_log("%s", "Stack error");\
    stackDump(__stk);              \
    return __VA_ARGS__;            \
}

#define stackCheckRetPtr(__stk, __errptr, ...)  \
if(stackError(__stk)){                \
    error_log("%s", "Stack error");   \
    stackDump(__stk);                 \
    if(__errptr)                      \
        *__errptr = stackError(__stk);\
    return __VA_ARGS__;               \
}

struct Stack{
    canary_t leftcan;

    ELEM_T *data;
    size_t size;
    size_t capacity;

    VarInfo info;
    hash_t data_hash;

    hash_t struct_hash;
    canary_t rightcan;
};

static hash_t stackGetDataHash(const Stack* stk){
    return gnuHash(stk->data, stk->data + stk->capacity);
}
static hash_t stackGetStructHash(const Stack* stk){
    return gnuHash((&(stk->leftcan))+sizeof(canary_t), &(stk->struct_hash));
}
static stackError_t stackUpdHashes(Stack* stk){
    if (stk == nullptr)
        return STACK_NULL;
    if (IsBadWritePtr(stk, sizeof(stk)))
        return STACK_BAD;
    if (stk->data == nullptr && stk->capacity != 0)
        return STACK_DATA_NULL;
    if (stk->data == DESTRUCT_PTR)
        return STACK_DATA_NULL;
    if (stk->size > stk->capacity)
        return STACK_SIZE_CAP_BAD;

    stk->data_hash   = stackGetDataHash  (stk);
    stk->struct_hash = stackGetStructHash(stk);

    return STACK_NOERROR;
}

static void stackCtor_(Stack* stk){
    stk->data = nullptr;
    stk->size = 0;
    stk->capacity = 0;

    stk->leftcan  = CANARY_L;
    stk->rightcan = CANARY_R;
}

#define stackCtor(__stk)    \
    if (__stk != nullptr){  \
        stackCtor_(__stk);      \
        (__stk)->info = varInfoInit(__stk); \
        stackUpdHashes(__stk);  \
    }                       \
    else {                  \
        error_log("%s", "nullptr passed to constructor\n");\
    }


static stackError_t stackError(const Stack* stk){
    if (stk == nullptr)
        return STACK_NULL;

    if (IsBadReadPtr(stk, sizeof(stk)))
        return STACK_BAD;

    unsigned int err = 0;

    if (stk->data == nullptr && stk->capacity != 0){
        err |= STACK_DATA_NULL;
    }

    if (stk->size == -1 || stk->capacity == -1 || stk->data == DESTRUCT_PTR){
        return STACK_DEAD;
    }

    if (stk->size > stk->capacity){
        err |= STACK_SIZE_CAP_BAD;
    }

    if (stk->leftcan != CANARY_L)
        err |= STACK_CANARY_L_BAD;
    if (stk->rightcan != CANARY_R)
        err |= STACK_CANARY_R_BAD;

    if (stk->struct_hash     != stackGetStructHash(stk)){
        err |= STACK_HASH_BAD;
    }

    if (stk->data != nullptr && !(err & STACK_HASH_BAD)){
        if (!checkLCanary(stk->data))
            err |= STACK_DATA_CANARY_L_BAD;
        if (!checkRCanary(stk->data, stk->capacity * sizeof(ELEM_T)))
            err |= STACK_DATA_CANARY_R_BAD;
    }

    if (!(err & STACK_DATA_NULL) && !(err & STACK_HASH_BAD)){
        if (stk->data_hash   != stackGetDataHash(stk)){
            err |= STACK_DATA_HASH_BAD;
        }
    }


    return (stackError_t)err;
}

static void stackDump(const Stack* stk){

    info_log("Stack dump:\n      stack at %p \n", stk);

    printf_log("      %ld/%ld elements\n", stk->size, stk->capacity);

    stackError_t err = stackError(stk);

    if (err & STACK_NULL){
        printf_log("(BAD)  Stack poiner is null\n");
        return;
    }
    if (err & STACK_BAD ){
        printf_log("(BAD)  Stack poiner is invalid\n");
        return;
    }


    printVarInfo_log(&(stk->info));

    if (err & STACK_CANARY_L_BAD){
        printf_log("      (BAD)  Struct L canary BAD! Value: %p\n", stk->leftcan);
    }
    if (err & STACK_CANARY_R_BAD){
        printf_log("      (BAD)  Struct R canary BAD! Value: %p\n", stk->rightcan);
    }

    if (err & STACK_DEAD){
        printf_log("      (BAD)  Stack was already destructed\n\n");
        return;
    }

    if (err & STACK_HASH_BAD){
        printf_log("      (BAD)  Structure hash invalid. Written %p calculated %p\n", stk->struct_hash, stackGetStructHash(stk));
        #ifdef NO_UNSAFE_PRINT
        return;
        #endif
    }


    if (stk->data == nullptr){
        printf_log("      (bad?) Stack data poiner is null\n\n");
        return;
    }

    if (err & STACK_DATA_BAD){
        printf_log("      (BAD)  Stack data poiner is invalid\n");
        return;
    }
    if (err & STACK_SIZE_CAP_BAD){
        printf_log("      (BAD)  Stack size is larger than capacity\n");
    }


    if(err & STACK_HASH_BAD){
        printf_log("      Danger: stack data hash is invalid, still trying to print data. segfaults may be.\n");
    }

    hash_t data_hash = stackGetDataHash(stk);
    if (data_hash != stk->data_hash){
        printf_log("      (BAD)  Data hash invalid. Written %p calculated %p\n", stk->data_hash  , stackGetDataHash(stk));
    }
    if(!checkLCanary(stk->data)){
        printf_log("      (BAD)  Data L canary BAD! Value: %p\n", ((canary_t*)stk->data)[-1]);
    }
    if(!checkRCanary(stk->data, stk->capacity * sizeof(ELEM_T))){
        printf_log("      (BAD)  Data R canary BAD! Value: %p\n",*((canary_t*)stk->data + stk->capacity));
    }

    printf_log("\n");

    for (int i = 0; i < stk->capacity; i++){
        printf_log("    ");

        printf_log("%c", (i < stk->size           ) ? '*':' ');

        printf_log("[%ld] " ELEM_SPEC " ", i, stk->data[i]);
        printf_log("%s", (stk->data[i] == BAD_ELEM) ? "(BAD)\n":"\n");
    }
    printf_log("\n");

}

static stackError_t stackDtor(Stack* stk){
    stackCheckRet(stk, stackError(stk));

    for (int i = 0; i < stk->capacity; i++){
        stk->data[i] = BAD_ELEM;
    }
    free(((canary_t*)stk->data) - 1);
    stk->data = DESTRUCT_PTR;
    stk->size = -1;
    stk->capacity = -1;
    (stk->info).status = VARSTATUS_DEAD;
}



static stackError_t stackResize(Stack* stk, size_t new_capacity){
    int err = stackError(stk);
    err &= ~(STACK_HASH_BAD | STACK_DATA_HASH_BAD);
    if (err)
        return (stackError_t)err;


    if (new_capacity < stk->size){
        return STACK_OP_INVALID;
    }

    errno = 0;
    ELEM_T* new_mem = nullptr;
    if (stk->data != nullptr){
        new_mem = (ELEM_T*)(
                            realloc(((canary_t*)stk->data) - 1, new_capacity*sizeof(ELEM_T) + 2*sizeof(canary_t))
                            + sizeof(canary_t));
    }
    else{
        new_mem = (ELEM_T*)(
                            calloc(                             new_capacity*sizeof(ELEM_T) + 2*sizeof(canary_t), 1)
                            + sizeof(canary_t));
    }
    if (new_mem == nullptr){
        perror_log("error while reallocating memory for stack");
        return STACK_OP_ERROR;
    }
    stk->data = new_mem;

    *((canary_t*)(stk->data + new_capacity)) = CANARY_R;
    *((canary_t*)(stk->data)-1)              = CANARY_L;

    for (int i = stk->capacity; i < new_capacity; i++){
        stk->data[i] = BAD_ELEM;
    }
    stk->capacity = new_capacity;
    return STACK_NOERROR;
}

static stackError_t stackPush(Stack* stk, ELEM_T elem){
    stackCheckRet(stk, stackError(stk));
    (stk->info).status = VARSTATUS_NORMAL;

    if (stk->size == stk->capacity){
        stackError_t err = stackResize(stk, (stk->capacity == 0)? STACK_MIN_SIZE : stk->capacity*2);
        if (err != STACK_NOERROR)
            return err;
    }

    stk->data[stk->size++] = elem;
    stackUpdHashes(stk);
    return stackError(stk);
}

static ELEM_T stackTop(Stack* stk, stackError_t *err_ptr = nullptr){
    stackCheckRetPtr(stk, err_ptr, BAD_ELEM);

    if (stk->size == 0){
        if (err_ptr)
            *err_ptr = STACK_OP_INVALID;
        return BAD_ELEM;
    }
    return stk->data[stk->size-1];
}

static ELEM_T stackPop(Stack* stk, stackError_t *err_ptr = nullptr){
    stackCheckRetPtr(stk, err_ptr, BAD_ELEM);
    (stk->info).status = VARSTATUS_NORMAL;

    if (stk->size == 0){
        if (err_ptr)
            *err_ptr = STACK_OP_INVALID;
        return BAD_ELEM;
    }

    ELEM_T ret = stk->data[--stk->size];

    stk->data[stk->size] = BAD_ELEM;

    if (stk->size * 2 < stk->capacity && stk->capacity > 2*STACK_MIN_SIZE){
        stackError_t err = stackResize(stk, (stk->capacity == 0)? STACK_MIN_SIZE : stk->size*2);
        if (err != STACK_NOERROR){
            if (err_ptr)
                *err_ptr = err;
            return BAD_ELEM;
        }

    }

    stackUpdHashes(stk);
    return ret;
}

