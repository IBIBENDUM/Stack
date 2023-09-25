#include <stdio.h>

#include "stack.h"

#define ABOBA

int main()
{
    stack stk;
    // init_stack(&stk);
    init_stack_with_capacity(&stk, 1);
    DEBUG_MSG("init_stack_with_capacity(&stk, 0);\n");
    DUMP_STACK(&stk, stdout);

    push_stack(&stk, 3);
    DEBUG_MSG("push_stack(&stk, 3);\n");
    DUMP_STACK(&stk, stdout);

    push_stack(&stk, 4);
    DEBUG_MSG("push_stack(&stk, 4);\n");
    DUMP_STACK(&stk, stdout);

    push_stack(&stk, 5);
    DEBUG_MSG("push_stack(&stk, 5);\n");
    DUMP_STACK(&stk, stdout);

    push_stack(&stk, 6);
    DEBUG_MSG("push_stack(&stk, 6);\n");
    DUMP_STACK(&stk, stdout);

    elem_t value = 0;
    pop_stack(&stk, &value);
    DUMP_STACK(&stk, stdout);
    pop_stack(&stk, &value);
    DUMP_STACK(&stk, stdout);
    pop_stack(&stk, &value);
    DUMP_STACK(&stk, stdout);
    pop_stack(&stk, &value);
    DUMP_STACK(&stk, stdout);
    pop_stack(&stk, &value);
    DUMP_STACK(&stk, stdout);

    destruct_stack(&stk);
}
