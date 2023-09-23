#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#define TAB "    " // Because \t is too big

// BAH: 1) Make log file
// BAH: 2) Make log file in html
// use minimum of html tag for better eye read

#define FREE_AND_NULL(PTR)\
do\
{\
    free(PTR);\
    PTR = NULL;\
} while(0)

#define DUMP_STACK(STK, FILE_PTR)\
do\
{\
    dump_stack(STK, FILE_PTR, __FILE__, __LINE__, __PRETTY_FUNCTION__);\
} while(0)

const ssize_t DEFAULT_STACK_SIZE = 5;
const int POISON_VALUE = INT_MAX;

/// Stack error codes
enum stack_error_code
{
    NO_ERROR,           ///< No error occurred
    NULL_STACK_POINTER, ///< Pointer on stack have NULL value
    NULL_DATA,          ///< Pointer on data have NULL value
    NEGATIVE_CAPACITY,  ///< Capacity is lower than zero
    NEGATIVE_SIZE,      ///< Size is lower than zero
    WRONG_SIZE          ///< Size is larger than capacity
};

typedef int elem_t;
#define ELEM_FORMAT "%d"

typedef struct STACK
{
    elem_t* data;
    ssize_t size; // BAH: ssize_t??
    ssize_t capacity;
} stack;

stack_error_code validate_stack(stack* stk) // BAH: Make error through bit operations
{
    if (!stk)                      return NULL_STACK_POINTER;
    if (!stk->data)                return NULL_DATA;
    if (stk->capacity < 0)         return NEGATIVE_CAPACITY;
    if (stk->size < 0)             return NEGATIVE_SIZE;
    if (stk->size > stk->capacity) return WRONG_SIZE;
    return NO_ERROR;
}

// struct for params
stack_error_code dump_stack(stack* stk, FILE* file_ptr, const char* file_name, const size_t line, const char* func_name)
{
    assert(stk);

    fprintf(file_ptr, "stack[%p] called from %s(%d) %s\n", stk, file_name, line, func_name);
    fprintf(file_ptr, "called from file_name func_name\n");
    fprintf(file_ptr, "{\n" TAB "size = %d\n" TAB "capacity = %d\n" TAB "data[%p]\n", stk->size, stk->capacity, stk);

    // BAH: make working good with bad stack
    // BAH: make type format
    fprintf(file_ptr, TAB "{\n");

    // // print every element
    // for (size_t i = 0; i < stk->capacity; i++)
    // {
    //     if (stk->data[i] != POISON_VALUE)
    //         fprintf(file_ptr, TAB TAB "*[%d] = %d\n", i, stk->data[i]);
    //     else
    //         fprintf(file_ptr, TAB TAB "[%d] = %d (POISON)\n", i, stk->data[i]);
    // }

    // print only non poison elements
    for (size_t i = 0; i < stk->capacity; i++)
    {
        while (i < stk->capacity && stk->data[i] != POISON_VALUE)
        {
            fprintf(file_ptr, TAB TAB "*[%d] = " ELEM_FORMAT "\n", i, stk->data[i]);
            i++;
        }
        if (i < stk->capacity && stk->data[i - 1] != POISON_VALUE)
            fprintf(file_ptr, TAB TAB "...\n");
    }

    fprintf(file_ptr, TAB "}\n");
    fprintf(file_ptr, "}\n");
}

bool check_for_stack_realloc(const stack* stk, ssize_t* new_capacity)
{
    const ssize_t size = stk->size;
    const ssize_t capacity = stk->capacity;

    if (size > capacity)
    {
        *new_capacity = size * 2;
        return true;
    }
    // if (4 * (size - 1) < capacity)
    // {
    //     *new_capacity = 2 * size;
    //     return true;
    // }

    return false;
}

stack_error_code realloc_stack(stack* stk, ssize_t new_capacity)
{
    // FROM INTERNET:
    // Note that your call to realloc
    // will mean you lose your data if,
    // for some reason the realloc fails

    stack* new_stk = (stack*) calloc(new_capacity, sizeof(new_stk[0]));
    memcpy(new_stk->data, stk->data, stk->size); // memset?
    memset(new_stk->data + stk->size, POISON_VALUE, new_capacity - stk->size);
}

stack_error_code push_stack(stack* stk, const elem_t value)
{
    stack_error_code error_code = validate_stack(stk);
    if (error_code != NO_ERROR)
    {
        DUMP_STACK(stk, stdout); // remake this
        return error_code;
    }
    stk->data[stk->size] = value;
    // handle error
    stk->size++;

    ssize_t new_capacity = 0;
    if (check_for_stack_realloc(stk, &new_capacity))
    {
        return realloc_stack(stk, new_capacity);
    }
    return NO_ERROR;
}

stack_error_code set_stack_with_poison(stack* stk)
{
    for (size_t i = 0; i < stk->capacity; i++)
    {
        stk->data[i] = POISON_VALUE;
    }
}

stack_error_code init_stack_with_capacity(stack* stk, ssize_t capacity) // BAH: Add overload?
{
    elem_t* data = (elem_t*) calloc(capacity, sizeof(data[0]));
    // if (err_code == NO_ERROR)
    // {
    stk->data = data;
    stk->size = 0;
    stk->capacity = capacity;
    // }
    set_stack_with_poison(stk);

    stack_error_code err_code = validate_stack(stk);

    return err_code;
}

stack_error_code init_stack(stack* stk)
{
    init_stack_with_capacity(stk, DEFAULT_STACK_SIZE);
}

stack_error_code destruct_stack(stack* stk)
{
    set_stack_with_poison(stk);
    FREE_AND_NULL(stk->data);
}

#endif
