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

#define RETURN_ERR_IF_STK_WRONG(PTR)\
    do {\
        unsigned error_bitmask = validate_stack(stk);\
        if (error_bitmask != NO_ERROR)\
        {\
            printf("TXLIB соболезнует: err_code %d", error_code);\
            abort();\
        }\
    } while(0)

#define DUMP_STACK(STK, FILE_PTR)\
    do {\
        struct dump_info INFO = { .file_name = __FILE__, \
                                  .line = __LINE__,\
                                  .func_name = __PRETTY_FUNCTION__\
                                };\
        dump_stack(STK, FILE_PTR, &INFO);\
    } while(0)

const ssize_t DEFAULT_STACK_SIZE = 8; // BAH: Make align to 8 bytes
const int POISON_VALUE = INT_MAX;

/// ` error codes
enum stack_error_code
{
    NO_ERROR,           ///< No error occurred
    NULL_STACK_POINTER, ///< Pointer on stack have NULL value
    NULL_DATA,          ///< Pointer on data have NULL value
    NULL_NEW_DATA,
    NULL_VALUE_PTR,
    DEAD_LEFT_SNITCH,
    DEAD_RIGHT_SNITCH,
    ANTI_OVERFLOW,
    NEGATIVE_CAPACITY,  ///< Capacity is lower than zero
    NEGATIVE_SIZE,      ///< Size is lower than zero
    WRONG_SIZE          ///< Size is larger than capacity
};

struct dump_info
{
    const char* file_name;
    const size_t line;
    const char* func_name;
};

// BAH: Make ability change element type from main (by define mb?)
#ifndef VALUE_TYPE
#define VALUE_TYPE int
#endif
typedef VALUE_TYPE elem_t;

#ifndef ELEM_FORMAT
#define ELEM_FORMAT "%d"
#endif

const long long snitch_value = 0xDEADB19D;
typedef struct STACK
{
    long long left_snitch = snitch_value;
    elem_t* data;
    ssize_t size;
    ssize_t capacity;
    long long right_snitch = snitch_value;
} stack;

static unsigned validate_stack(stack* stk) // BAH: Make error through bit operations
{
    unsigned error_bitmask = 0;
    if (!stk)                              return NULL_STACK_POINTER;
    if (!stk->data)                        return NULL_DATA;
    if (stk->left_snitch  != snitch_value) return DEAD_LEFT_SNITCH;
    if (stk->right_snitch != snitch_value) return DEAD_RIGHT_SNITCH;
    if (stk->capacity < 0)                 return NEGATIVE_CAPACITY;
    if (stk->size     < 0)                 return NEGATIVE_SIZE;
    if (stk->size     > stk->capacity)     return WRONG_SIZE;

    return NO_ERROR;
}

stack_error_code dump_stack(stack* stk, FILE* file_ptr, struct dump_info* info)
{
    if(!stk)
        return NULL_STACK_POINTER;

    assert(file_ptr);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    // BAH: I know about %p, but where was strange output with leading zeroes like 000004F856CD
    fprintf(file_ptr, "\nstack[0x%X] called from %s(%d) %s\n", stk, info->file_name, info->line, info->func_name);
    fprintf(file_ptr, "called from file_name func_name\n");
    fprintf(file_ptr, "{\n" TAB "size = %d\n" TAB "capacity = %d\n" TAB "data[0x%X]\n", stk->size, stk->capacity, stk->data);

    fprintf(file_ptr, TAB "{\n");
    if (stk->capacity < 1)
    {
        fprintf(file_ptr, TAB TAB "NULL\n");
    }
    else
    {
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
    }
    fprintf(file_ptr, TAB "}\n");
    fprintf(file_ptr, "}\n");

    return NO_ERROR;
}

static bool check_for_stack_realloc(const stack* stk, ssize_t* const new_capacity)
{
    assert(new_capacity);

    if (!stk)
        return NULL_STACK_POINTER;

    const ssize_t size = stk->size;
    const ssize_t capacity = stk->capacity;

    if (size + 1 > capacity)
    {
        *new_capacity = size * 2;
        return true;
    }

    if (4 * size < capacity)
    {
        *new_capacity = capacity / 2;
        return true;
    }

    return false;
}

static stack_error_code fill_data_with_poison(elem_t* data_ptr, size_t size)
{
    if (!data_ptr)
        return NULL_DATA;

    if (size < 0)
        return WRONG_SIZE;

    for (size_t i = 0; i < size; i++)
    {
        assert(0 <= i && i < size);

        data_ptr[i] = POISON_VALUE;
    }

    return NO_ERROR;
}

static stack_error_code realloc_stack(stack* stk, const ssize_t new_capacity)
{
    if (!stk)
        return NULL_STACK_POINTER;

    if (new_capacity < stk->size)
        return WRONG_SIZE;

    // Realloc vs calloc + memcpy?
    elem_t* new_data = (elem_t*) realloc(stk->data, new_capacity * sizeof(new_data[0]));

    if (new_data)
    {
        fill_data_with_poison(new_data + stk->size, new_capacity - stk->size);
        stk->data = new_data;
        stk->capacity = new_capacity;

        return NO_ERROR;
    }

    return NULL_NEW_DATA;
}

stack_error_code push_stack(stack* stk, const elem_t value)
{
    RETURN_ERR_IF_STK_WRONG(stk);

    stk->data[stk->size++] = value;

    ssize_t new_capacity = 0;
    if (check_for_stack_realloc(stk, &new_capacity))
    {
        return realloc_stack(stk, new_capacity);
    }

    return NO_ERROR;
}

stack_error_code pop_stack(stack* stk, elem_t* const value)
{
    RETURN_ERR_IF_STK_WRONG(stk);

    if (value)
    {
        if (stk->size - 1 >= 0)
        {
            *value = stk->data[--stk->size];
            stk->data[stk->size] = POISON_VALUE;
            ssize_t new_capacity = 0;
            if (check_for_stack_realloc(stk, &new_capacity))
            {
                return realloc_stack(stk, new_capacity);
            }
        }

        return ANTI_OVERFLOW;
    }

    return NULL_VALUE_PTR;
}

stack_error_code init_stack_with_capacity(stack* stk, ssize_t capacity) // BAH: Add overload?
{
    // Make align bytes
    if (stk)
    {
        if (capacity < 1) // Remake this
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

    return NULL_STACK_POINTER;
}

stack_error_code init_stack(stack* stk)
{
    if (stk)
    {
        init_stack_with_capacity(stk, DEFAULT_STACK_SIZE);
        return NO_ERROR;
    }

    return NULL_STACK_POINTER;
}

stack_error_code destruct_stack(stack* stk)
{
    RETURN_ERR_IF_STK_WRONG(stk);

    fill_data_with_poison(stk->data, stk->capacity);
    FREE_AND_NULL(stk->data);

    return NO_ERROR;
}

#endif
