#include <stdio.h>

#include "stack.h"

int main()
{
    stack stk;
    // init_stack(&stk);
    init_stack_with_capacity(&stk, 0);
    DUMP_STACK(&stk, stdout);
    push_stack(&stk, 3);
    DUMP_STACK(&stk, stdout);
    push_stack(&stk, 4);
    DUMP_STACK(&stk, stdout);

    destruct_stack(&stk);
}
