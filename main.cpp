#include <stdio.h>

#define VALUE_TYPE int
#define ELEM_FORMAT "%d"
#include "stack.h"
#undef VALUE_TYPE
#undef ELEM_FORMAT

int main()
{
    stack stk;
    stack* stk_ptr = &stk;

    // init_stack(stk_ptr);
    INIT_STACK(stk, 1);
    DEBUG_MSG("INIT_STACK(stk, 0);\n");
    DUMP_STACK(stk_ptr, stdout);

    push_stack(stk_ptr, 3);
    DEBUG_MSG("push_stack(stk_ptr, 3);\n");
    DUMP_STACK(stk_ptr, stdout);


    push_stack(stk_ptr, 4);
    DEBUG_MSG("push_stack(stk_ptr, 4);\n");
    DUMP_STACK(stk_ptr, stdout);

    // for (size_t i = 0; i < sizeof(stk); i++)
    //     ((char*) stk_ptr)[i] = 0;

    push_stack(stk_ptr, 5);
    DEBUG_MSG("push_stack(stk_ptr, 5);\n");
    DUMP_STACK(stk_ptr, stdout);

    push_stack(stk_ptr, 6);
    DEBUG_MSG("push_stack(stk_ptr, 6);\n");
    DUMP_STACK(stk_ptr, stdout);

//     elem_t value = 0;
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stk_ptr, stdout);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stk_ptr, stdout);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stk_ptr, stdout);
//     pop_stack(stk_ptr, &value);
//     DUMP_STACK(stk_ptr, stdout);
//     pop_stack(stk_ptr, &value);
    // DUMP_STACK(stk_ptr, stdout);

    destruct_stack(stk_ptr);
    // DUMP_STACK(stk_ptr, stdout);

}
