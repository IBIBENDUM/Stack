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

    open_html();

    init_stack(stk);
    push_stack(stk_ptr, 63);
    push_stack(stk_ptr, 64);

    int value = 0;
    pop_stack(stk_ptr, &value);


    destruct_stack(stk_ptr);

    close_html();
    return 0;
}
