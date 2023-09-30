#include <stdio.h>

// #define VALUE_TYPE char
// #define ELEM_FORMAT "%c"
// #define POISON_VAL 10
#include "stack.h"
// #undef VALUE_TYPE
// #undef ELEM_FORMAT
// #undef POISON_VAL
int main()
{
    stack stk;
    stack* stk_ptr = &stk;

    init_stack(stk);
    DEBUG_MSG("INIT_STACK(stk, 0);\n");
    dump_stack(stdout, stk_ptr, 0);

    push_stack(stk_ptr, 63);
    DEBUG_MSG("push_stack(stk_ptr, 3);\n");
    dump_stack(stdout, stk_ptr, 0);

    log_stack_to_html(stk_ptr);

    stk.size = -1;
    stk.capacity = -1;

    push_stack(stk_ptr, 64);
    DEBUG_MSG("push_stack(stk_ptr, 4);\n");
    dump_stack(stdout, stk_ptr, 0);

    // stk.capacity = 1000000;
    destruct_stack(stk_ptr);
    dump_stack(stdout, stk_ptr, 0);

    close_html();
    return 0;
}
