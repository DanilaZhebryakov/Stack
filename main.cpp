#define NDEBUG

#include <stdio.h>

#include "Stack.h"

using namespace std;

int main()
{
    Stack stk;
    stackCtor(&stk);

    stackResize(&stk, 30);
    stackDump(&stk);

    for (int i = 0; i < 21; i++){
        stackPush(&stk, 56+i);
    }

    stackDump(&stk);

    for (int i = 0; i < 21; i++){
        printf("%d\n",  stackPop(&stk));
    }
    stackDump(&stk);
    stackDtor(&stk);
    return 0;
}
