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

// what is local define???

// Obfuscation??

#ifdef DEBUG
    #define DEBUG_MSG(FORMAT, ...)\
        do {\
            printf(FORMAT, ##__VA_ARGS__); /* BAH: "##" because of empty __VA_ARGS__*/ \
        } while (0)
#else
    #define DEBUG_MSG(FORMAT, ...)
#endif

#define FREE_AND_NULL(PTR)\
    do {\
        free(PTR);\
        PTR = NULL;\
    } while(0)

#define DUMP_STACK(STK, FILE_PTR)\
    do {\
        struct file_info INFO = { .file_name = __FILE__, \
                                  .line = __LINE__,\
                                  .func_name = __PRETTY_FUNCTION__\
                                };\
        dump_stack(STK, FILE_PTR, &INFO);\
    } while(0)

const ssize_t DEFAULT_STACK_SIZE = 5; // BAH: Make align to 8 bytes
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

struct file_info
{
    const char* file_name;
    const size_t line;
    const char* func_name;
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
stack_error_code dump_stack(stack* stk, FILE* file_ptr, struct file_info* info)
{
    assert(stk);
    assert(file_ptr);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    // BAH: I know about %p, but where was strange output with leading zeroes like 000004F856CD
    fprintf(file_ptr, "\nstack[0x%X] called from %s(%d) %s\n", stk, info->file_name, info->line, info->func_name);
    fprintf(file_ptr, "called from file_name func_name\n");
    fprintf(file_ptr, "{\n" TAB "size = %d\n" TAB "capacity = %d\n" TAB "data[0x%X]\n", stk->size, stk->capacity, stk->data);

    // BAH: make working good with bad stack
    fprintf(file_ptr, TAB "{\n");
    if (!stk->capacity)
    {
        fprintf(file_ptr, TAB TAB "NULL\n");
    }

    // // Print every element
    // for (size_t i = 0; i < stk->capacity; i++)
    // {
    //     if (stk->data[i] != POISON_VALUE)
    //         fprintf(file_ptr, TAB TAB "*[%d] = %d\n", i, stk->data[i]);
    //     else
    //         fprintf(file_ptr, TAB TAB "[%d] = %d (POISON)\n", i, stk->data[i]);
    // }

    // Print only non poison elements
    for (size_t i = 0; i < stk->capacity; i++)
    {
        while (i < stk->capacity && stk->data[i] != POISON_VALUE)
        {
            fprintf(file_ptr, TAB TAB "*[%d] = " ELEM_FORMAT "\n", i, stk->data[i]);
            i++;
        }
        if (i < stk->capacity && stk->data[i - 1] != POISON_VALUE)
            fprintf(file_ptr, TAB TAB "... (POISON)\n");
    }

    fprintf(file_ptr, TAB "}\n");
    fprintf(file_ptr, "}\n");

    return NO_ERROR;
}

bool check_for_stack_realloc(const stack* stk, ssize_t* const new_capacity)
{
    assert(stk);
    assert(new_capacity);

    const ssize_t size = stk->size;
    const ssize_t capacity = stk->capacity;

    if (size + 1 > capacity)
    {
        *new_capacity = size * 2;
        return true;
    }
//
//     if (2 * size < capacity)
//     {
//         *new_capacity = 2 * size;
//         return true;
//     }

    return false;
}

stack_error_code fill_data_with_poison(elem_t* data_ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        data_ptr[i] = POISON_VALUE;
    }

    return NO_ERROR;
}

stack_error_code realloc_stack(stack* stk, const ssize_t new_capacity)
{
    assert(stk);
    assert(new_capacity);

    // FROM INTERNET:
    // Note that your call to realloc
    // will mean you lose your data if,
    // for some reason the realloc fails

    // Or realloc?
    elem_t* new_data = (elem_t*) calloc(new_capacity, sizeof(new_data[0]));
    // Handle errors
    // free and poison and null

    // Use memcpy_s?
    for (size_t i = 0; i < stk->size; i++)
    {
        DEBUG_MSG("stk->data[%d] = %d\n", i, stk->data[i]);
    }
    memcpy(new_data, stk->data, stk->size * sizeof(elem_t));

    DEBUG_MSG("offset = %d\n", stk->size);
    DEBUG_MSG("new_capacity = %d\n", new_capacity);
    // DEBUG_MSG("size until end = %d\n", new_capacity - stk->size);

    fill_data_with_poison(new_data + stk->size, new_capacity - stk->size);

    stk->data = new_data;
    stk->capacity = new_capacity;

    return NO_ERROR;

    // Handle errors
}

stack_error_code push_stack(stack* stk, const elem_t value)
{
    stack_error_code error_code = validate_stack(stk);
    if (error_code != NO_ERROR)
    {
        return error_code;
    }

    // DEBUG_MSG("stk->size = %d\n", stk->size);
    stk->data[stk->size++] = value;
    // DEBUG_MSG("after: stk->size = %d\n", stk->size);
    ssize_t new_capacity = 0;
    if (check_for_stack_realloc(stk, &new_capacity))
    {
        return realloc_stack(stk, new_capacity);
    }

    return NO_ERROR;
}

stack_error_code pop_stack(stack* stk, elem_t* const value)
{
    stack_error_code error_code = validate_stack(stk);
    if (error_code != NO_ERROR)
    {
        return error_code;
    }
    --stk->size;
    // check for stack anti-overflow
    *value = stk->data[stk->size];
    stk->data[stk->size] = POISON_VALUE;

    return NO_ERROR;
}

stack_error_code init_stack_with_capacity(stack* stk, ssize_t capacity) // BAH: Add overload?
{
    if (capacity == 0)
        capacity = 1;
    elem_t* data = (elem_t*) calloc(capacity, sizeof(data[0]));

    if (data)
    {
        stk->data = data;
        stk->size = 0;
        stk->capacity = capacity;
        fill_data_with_poison(stk->data, stk->capacity);
        stack_error_code err_code = validate_stack(stk);

        return err_code;
    }

    return NULL_DATA;
}

stack_error_code init_stack(stack* stk)
{
    init_stack_with_capacity(stk, DEFAULT_STACK_SIZE);
    return NO_ERROR;
}

stack_error_code destruct_stack(stack* stk)
{
    fill_data_with_poison(stk->data, stk->capacity);
    FREE_AND_NULL(stk->data);
    return NO_ERROR;
}

#endif
