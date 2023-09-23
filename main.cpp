#include <stdio.h>

#include "stack.h"

int main()
{
    stack stk;
    // init_stack(&stk);
    init_stack_with_capacity(&stk, 1);
    push_stack(&stk, 3);
    push_stack(&stk, 4);
    push_stack(&stk, 5);

    stk.data[6] = 9;
    DUMP_STACK(&stk, stdout);

    destruct_stack(&stk);
}
