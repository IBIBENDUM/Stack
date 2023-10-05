#include <stdio.h>
#include "stack.h"
#include "stack_logs.h"
int main()
{
    stack stk = {};
    stack* stk_ptr = &stk;

    open_log_file();

    init_stack(stk);
    push_stack(stk_ptr, 63);
    push_stack(stk_ptr, 64);

    int value = 0;
    stk_ptr = NULL;
    pop_stack(stk_ptr, &value);
    pop_stack(stk_ptr, &value);


    destruct_stack(stk_ptr);

    close_log_file();
    printf("Finish!\n");
    return 0;
}
