#include <stdio.h>

#define VALUE_TYPE int
#define ELEM_FORMAT "%d"
#include "stack.h"
#undef VALUE_TYPE
#undef ELEM_FORMAT

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

    stack* stk_ptr = &stk;
    for (size_t i = 0; i < 40; i++)
        ((char*) stk_ptr)[i] = 0;

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
