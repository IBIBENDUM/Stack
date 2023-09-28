#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#define TAB "    " // Because \t is too big

const size_t BYTE_ALIGN = 8;
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
        unsigned error_bitmask = validate_stack(PTR);\
        if (error_bitmask)\
        {\
            printf("Error occurred:\n");\
            print_errors(error_bitmask);\
            DUMP_STACK(PTR, stdout);\
            assert(0);\
            /* return error_bitmask; */ \
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

#define INIT_STACK(STK, CAPACITY)\
    do {\
        struct initialize_info INFO = { .var_name = #STK,\
                                        .file_name = __FILE__,\
                                        .line = __LINE__,\
                                        .func_name = __PRETTY_FUNCTION__\
                                      };\
        init_stack_with_capacity(&STK, CAPACITY, &INFO);\
    } while(0)

const ssize_t DEFAULT_STACK_SIZE = 8; // BAH: Make align to 8 bytes
const int POISON_VALUE = INT_MAX;

#define STACK_ERRORS\
  INIT_ERROR(NO_ERROR)\
  INIT_ERROR(NULL_STACK_POINTER)\
  INIT_ERROR(NULL_DATA)\
  INIT_ERROR(NULL_NEW_DATA)\
  INIT_ERROR(DEAD_LEFT_SNITCH)\
  INIT_ERROR(DEAD_RIGHT_SNITCH)\
  INIT_ERROR(WRONG_STRUCT_HASH)\
  INIT_ERROR(WRONG_DATA_HASH)\
  INIT_ERROR(DEAD_LEFT_DATA_SNITCH)\
  INIT_ERROR(DEAD_RIGHT_DATA_SNITCH)\
  INIT_ERROR(NEGATIVE_CAPACITY)\
  INIT_ERROR(NEGATIVE_SIZE)\
  INIT_ERROR(WRONG_SIZE)\
  INIT_ERROR(NULL_VALUE_PTR)\
  INIT_ERROR(ANTI_OVERFLOW)

enum stack_error_code
{
    #define INIT_ERROR(ERR_CODE) ERR_CODE,
        STACK_ERRORS
    #undef INIT_ERROR
};

struct initialize_info
{
    const char* var_name;
    const char* file_name;
    ssize_t line;
    const char* func_name;
};

struct dump_info
{
    const char* file_name;
    const size_t line;
    const char* func_name;
};

#ifndef VALUE_TYPE
#define VALUE_TYPE int
#endif
typedef VALUE_TYPE elem_t;

#ifndef ELEM_FORMAT
#define ELEM_FORMAT "%d"
#endif


unsigned get_hash(const void* key, size_t len)
{
    assert(key);
    assert(len > 0);

    const unsigned seed = 0xACAB1337;
    const unsigned MULTIPLY_VAL = 0xDED1DEAD;   // DED != DEAD
    const int ROTATE_VAL = 24;
    unsigned hash = seed ^ len;

    const unsigned char* data = (const unsigned char *) key;

    const size_t BYTES_IN_CHUNK = 4;
    while(len >= BYTES_IN_CHUNK)
    {
        unsigned chunk = *(unsigned*)data;

        chunk *= MULTIPLY_VAL;
        chunk ^= chunk >> ROTATE_VAL;
        chunk *= MULTIPLY_VAL;

        hash *= MULTIPLY_VAL;
        hash ^= chunk;

        data += BYTES_IN_CHUNK;
        len -= BYTES_IN_CHUNK;
    }

    switch(len)
    {
        case  3:  hash ^= data[2] << 16;
        case  2:  hash ^= data[1] <<  8;
        case  1:  hash ^= data[0];
        default:  hash *= MULTIPLY_VAL;
    };

    hash ^= hash >> 13;
    hash *= MULTIPLY_VAL;
    hash ^= hash >> 15;

    return hash;
}

const unsigned long long SNITCH_VALUE = 0xABADC0DEDA551337;     // A BAD CODED ASS 1337

typedef struct STACK
{
    unsigned long long left_snitch = SNITCH_VALUE;
    struct initialize_info init_info;
    elem_t* data;
    ssize_t size;
    ssize_t capacity;
    unsigned struct_hash;
    unsigned data_hash;
    unsigned long long right_snitch = SNITCH_VALUE;
} stack;

void print_errors(const unsigned val)
{
    for (size_t i = 0; i < sizeof(val) * 8; i++)
    {
        if (val & (1 << i))
        {
            switch (i)
            {
                #define INIT_ERROR(ERR_NAME)\
                    case ERR_NAME: printf("%s\n", #ERR_NAME); break;
                    STACK_ERRORS
                #undef INIT_ERROR
                #undef STACK_ERRORS
                default: break;
            }
        }
    }
}

unsigned get_stack_hash(stack* stk)
{
    unsigned old_struct_hash = stk->struct_hash;
    stk->struct_hash = 0;

    unsigned old_data_hash = stk->data_hash;
    stk->data_hash = 0;

    unsigned new_struct_hash = get_hash(stk, sizeof(stack));

    stk->struct_hash = old_struct_hash;
    stk->data_hash = old_data_hash;

    return new_struct_hash;
}

static unsigned validate_stack(stack* stk)
{
    unsigned error_bitmask = 0;

    if (!stk)                                                        error_bitmask |= 1 << NULL_STACK_POINTER;
    if (!stk->data)                                                  error_bitmask |= 1 << NULL_DATA;
    if (stk->left_snitch  != SNITCH_VALUE)                           error_bitmask |= 1 << DEAD_LEFT_SNITCH;
    if (stk->right_snitch != SNITCH_VALUE)                           error_bitmask |= 1 << DEAD_RIGHT_SNITCH;
    if (stk->capacity     < 0)                                       error_bitmask |= 1 << NEGATIVE_CAPACITY;
    if (stk->size         < 0)                                       error_bitmask |= 1 << NEGATIVE_SIZE;
    if (stk->size         > stk->capacity)                           error_bitmask |= 1 << WRONG_SIZE;
    if (*((long long*) stk->data - 1)
        != SNITCH_VALUE)                                             error_bitmask |= 1 << DEAD_LEFT_DATA_SNITCH;
    if (*(long long*)(stk->data + stk->capacity)
        != SNITCH_VALUE)                                             error_bitmask |= 1 << DEAD_RIGHT_DATA_SNITCH;

    if (stk) // check hash only if other checks went correct
    {
        if (stk->struct_hash  != get_stack_hash(stk))                error_bitmask |= 1 << WRONG_STRUCT_HASH;
        if (stk->data_hash    != get_hash(stk->data,
                              stk->capacity * sizeof(elem_t)))       error_bitmask |= 1 << WRONG_DATA_HASH;
    }

    return error_bitmask;
}

stack_error_code dump_stack(stack* stk, FILE* file_ptr, struct dump_info* info)
{
    assert(file_ptr);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    fprintf(file_ptr, "\nstack[0x%llX] \"%s\" initialized from %s(%d) %s\n", stk, stk->init_info.var_name, stk->init_info.file_name,
                                                                              stk->init_info.line, stk->init_info.func_name);
    fprintf(file_ptr, "called from %s(%d) %s\n", info->file_name, info->line, info->func_name);

    if(!stk)
        return NULL_STACK_POINTER;

    fprintf(file_ptr, "struct_left_snitch = 0x%llX\n", stk->left_snitch);
    fprintf(file_ptr, "{\n" TAB "size = %d\n" TAB "capacity = %d\n" TAB "data[0x%llX]\n", stk->size, stk->capacity, stk->data);


    if (stk->data)
    {
        fprintf(file_ptr, TAB "{\n");
        fprintf(file_ptr, TAB TAB "data_left_snitch = 0x%llX\n", *(unsigned long long*)((char*)stk->data - sizeof(long long)));
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
        fprintf(file_ptr, TAB TAB "data_right_snitch = 0x%llX\n", *(unsigned long long*)(stk->data + stk->capacity));

        fprintf(file_ptr, TAB "}\n");

    }

    fprintf(file_ptr, "}\n");
    fprintf(file_ptr, "struct_right_snitch = 0x%llX\n", stk->right_snitch);
    fprintf(file_ptr, "struct_hash = 0x%llX\n", stk->struct_hash);
    fprintf(file_ptr, "data_hash = 0x%llX\n", stk->data_hash);

    if (!stk->data) return NULL_DATA;
    return NO_ERROR;
}

void paste_snitch_value(void* data_void)
{
    long long* data = (long long*) data_void;
    data[0] = SNITCH_VALUE;
}

static stack_error_code calculate_new_capacity(const stack* stk, ssize_t* const new_capacity)
{
    assert(new_capacity);

    if (!stk)
        return NULL_STACK_POINTER;

    const ssize_t size = stk->size;
    const ssize_t capacity = stk->capacity;

    if (size + 1 > capacity)
    {
        *new_capacity = capacity * 2;
    }

    else if (4 * size < capacity)
    {
        *new_capacity = capacity / 2;
    }

    return NO_ERROR;
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

void update_stack_hash(stack* stk)
{
    assert(stk);

    stk->struct_hash = get_stack_hash(stk);
    stk->data_hash   = get_hash(stk->data, stk->capacity * sizeof(elem_t));
}

static stack_error_code realloc_stack(stack* stk, const ssize_t new_capacity)
{
    if (!stk)
        return NULL_STACK_POINTER;

    if (new_capacity < stk->size)
        return WRONG_SIZE;

    // Realloc vs calloc + memcpy?
    *(long long*)(stk->data + stk->capacity) = 0;
    elem_t* new_data = (elem_t*) realloc((char*)stk->data - sizeof(long long), new_capacity * sizeof(new_data[0]) + 2 * sizeof(long long));

    if (new_data)
    {
        elem_t* data_ptr = new_data + sizeof(long long) / sizeof(new_data[0]);
        fill_data_with_poison(data_ptr + stk->size, new_capacity - stk->size);
        stk->data = data_ptr;
        stk->capacity = new_capacity;
        paste_snitch_value(stk->data + stk->capacity);

        update_stack_hash(stk);

        return NO_ERROR;
    }

    return NULL_NEW_DATA;
}

unsigned push_stack(stack* stk, const elem_t value)
{
    RETURN_ERR_IF_STK_WRONG(stk);

    stk->data[stk->size++] = value;

    ssize_t new_capacity = 0;
    calculate_new_capacity(stk, &new_capacity);
    if (new_capacity)
    {
        return 0 | 1 << realloc_stack(stk, new_capacity);
    }

    update_stack_hash(stk);

    return 0 | 1 << NO_ERROR;
}

unsigned pop_stack(stack* stk, elem_t* const value)
{
    RETURN_ERR_IF_STK_WRONG(stk);

    if (value)
    {
        if (stk->size - 1 >= 0)
        {
            *value = stk->data[--stk->size];
            stk->data[stk->size] = POISON_VALUE;
            ssize_t new_capacity = 0;
            calculate_new_capacity(stk, &new_capacity);
            if (new_capacity)
            {
                return realloc_stack(stk, new_capacity);
            }

            update_stack_hash(stk);
        }

        return 0 | 1 << ANTI_OVERFLOW;
    }

    return 0 | 1 << NULL_VALUE_PTR;
}

ssize_t align_capacity(const ssize_t capacity)
{
    size_t bytes_count = capacity * sizeof(elem_t);
    return (bytes_count + BYTE_ALIGN - bytes_count % BYTE_ALIGN) / sizeof(elem_t);
}

stack_error_code init_stack_with_capacity(stack* stk, ssize_t capacity, struct initialize_info* info)
{
    assert(info);
    assert(info->var_name);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    if (stk)
    {
        if (capacity < 1)
            capacity = 1;

        capacity = align_capacity(capacity);

        const size_t data_size_in_bytes = capacity + 2 * sizeof(long long) / sizeof(elem_t);
        elem_t* data = (elem_t*) calloc(data_size_in_bytes, sizeof(data[0]));
        if (data)
        {
            stk->data     = (elem_t*) ((char*)data + sizeof(long long));
            stk->size     = 0;
            stk->capacity = capacity;


            paste_snitch_value(data);
            paste_snitch_value(stk->data + capacity);
            fill_data_with_poison(stk->data, stk->capacity);

            stk->init_info.var_name  = info->var_name;
            stk->init_info.file_name = info->file_name;
            stk->init_info.line      = info->line;
            stk->init_info.func_name = info->func_name;

            stk->struct_hash = get_stack_hash(stk);
            stk->data_hash = get_hash(stk->data, capacity * sizeof(elem_t));
        }

        return NULL_DATA;
    }

    return NULL_STACK_POINTER;
}

// stack_error_code init_stack(stack* stk)
// {
//     if (stk)
//     {
//         init_stack_with_capacity(stk, DEFAULT_STACK_SIZE);
//         return NO_ERROR;
//     }
//
//     return NULL_STACK_POINTER;
// }

stack_error_code destruct_stack(stack* stk)
{
    if (stk)
    {
        if (stk->data)
        {
            elem_t* data_ptr = (elem_t*)((char*)stk->data - sizeof(long long));
            if (stk->capacity > 0)
            {
                fill_data_with_poison(data_ptr, stk->capacity);
                FREE_AND_NULL(data_ptr);
                return NO_ERROR;
            }
            return NEGATIVE_CAPACITY;
        }
        return NULL_DATA;
    }
    return NULL_STACK_POINTER;
}

#endif
