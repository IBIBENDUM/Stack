#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "stack.h"
#include "stack_logs.h"

#define FREE_AND_NULL(PTR)\
    do {\
        free(PTR);\
        PTR = NULL;\
    } while(0)

#define RETURN_ERR_IF_STK_WRONG(STK)\
    do {\
        unsigned ERROR_BITMASK = validate_stack(STK);\
        if (ERROR_BITMASK)\
        {\
            dump_stack(stderr, STK, ERROR_BITMASK);\
            log_stack_to_file(STK);\
            close_log_file();\
            assert(0);\
            return ERROR_BITMASK;\
        }\
    } while(0)

#define LOG_STACK(STK)\
    do {\
        unsigned ERROR_BITMASK = validate_stack(STK);\
        dump_stack(stderr, STK, ERROR_BITMASK);\
        log_stack_to_file(STK);\
    } while(0)

const size_t BYTE_ALIGN = 8; ///< @see align_stack_size()
const size_t STACK_CAPACITY_MULTIPLIER = 2; ///< Capacity multiplier @see calculate_new_capacity()
const ssize_t DEFAULT_STACK_SIZE = 1; ///< Stack size on initialization @see init_stack()

static stack_error_code fill_data_with_poison(elem_t* data_ptr, size_t size);
static ssize_t align_stack_size(ssize_t size);
static stack_error_code calculate_new_capacity(const stack* stk, ssize_t* const new_capacity);
static stack_error_code realloc_stack(stack* stk, const ssize_t new_capacity);
static void paste_snitch_value(void* data_void);
static void print_separator(FILE* file_ptr);
static void print_errors(FILE* file_ptr, const unsigned error_bitmask);
static unsigned get_hash(void* key, size_t len);
static unsigned get_stack_hash(stack* stk);
static void update_stack_hash(stack* stk);

/// @return Bitmask that contains all found errors
unsigned validate_stack(stack* stk)
{
    unsigned error_bitmask = 0;
    if (!stk)
    {
        error_bitmask |= 1 << NULL_STACK_POINTER;
        return error_bitmask;
    }

    if (stk->size     < 0)
        error_bitmask |= 1 << NEGATIVE_SIZE;

    if (stk->size     > stk->capacity)
        error_bitmask |= 1 << WRONG_SIZE;

    if (!stk->data)
        error_bitmask |= 1 << NULL_DATA;
    #ifdef SNITCH
    if (stk->left_snitch  != SNITCH_VALUE)
        error_bitmask |= 1 << DEAD_LEFT_SNITCH;

    if (stk->right_snitch != SNITCH_VALUE)
        error_bitmask |= 1 << DEAD_RIGHT_SNITCH;
    #endif

    if (stk->capacity < 1)
    {
        error_bitmask |= 1 << NEGATIVE_CAPACITY;
        return error_bitmask;
    }

    #ifdef HASH
    if (stk->struct_hash  != get_stack_hash(stk))
    {
        error_bitmask |= 1 << WRONG_STRUCT_HASH;
        return error_bitmask;
    }
    if (stk->data_hash    != get_hash(stk->data, stk->capacity * sizeof(elem_t)))
    {
        error_bitmask |= 1 << WRONG_DATA_HASH;
        return error_bitmask;
    }
    #endif

    #ifdef SNITCH
    if (stk->data)
    {
        if (*((snitch_t*) stk->data - 1) != SNITCH_VALUE)
            error_bitmask |= 1 << DEAD_LEFT_DATA_SNITCH;
        if (*(snitch_t*)(stk->data + stk->capacity) != SNITCH_VALUE)
            error_bitmask |= 1 << DEAD_RIGHT_DATA_SNITCH;
    }
    #endif

    return error_bitmask;
}
/// @brief Fill data_ptr with POISON_VALUE
static stack_error_code fill_data_with_poison(elem_t* data_ptr, size_t size)
{
    assert(data_ptr);

    for (size_t i = 0; i < size; i++)
        data_ptr[i] = POISON_VALUE;

    return NO_ERROR;
}

/// @brief Align size to a multiple of 8 bytes
/// @return Aligned value
static ssize_t align_stack_size(ssize_t size)
{
    return (sizeof(elem_t) * size + BYTE_ALIGN - sizeof(elem_t) * size % BYTE_ALIGN) / sizeof(elem_t);
}

/// @brief Initialize stack with poison value@n
///        Should be used through macro init_stack
stack_error_code (init_stack)(stack* stk, struct initialize_info* info)
{
    assert(info);
    assert(info->var_name);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    if (stk)
    {
        ssize_t capacity = align_stack_size(DEFAULT_STACK_SIZE);
        const size_t data_size = capacity IF_SNITCH_ON(+ 2 * sizeof(snitch_t) / sizeof(elem_t));
        elem_t* data = (elem_t*) calloc(data_size, sizeof(data[0]));

        if (data)
        {
            stk->data = (elem_t*)((char*)data IF_SNITCH_ON(+ sizeof(snitch_t)));

            stk->size     = 0;
            stk->capacity = capacity;

            #ifdef SNITCH
            paste_snitch_value(data);
            paste_snitch_value(stk->data + capacity);
            #endif

            fill_data_with_poison(stk->data, stk->capacity);

            stk->init_info.var_name  = info->var_name;
            stk->init_info.file_name = info->file_name;
            stk->init_info.line      = info->line;
            stk->init_info.func_name = info->func_name;

            #ifdef HASH
            stk->struct_hash = get_stack_hash(stk);
            stk->data_hash   = get_hash(stk->data, capacity * sizeof(elem_t));
            #endif
        }
        return NULL_DATA;
    }
    return NULL_STACK_POINTER;
}

static stack_error_code calculate_new_capacity(const stack* stk, ssize_t* const new_capacity)
{
    assert(new_capacity);
    assert(stk);

    const ssize_t size = stk->size;
    const ssize_t capacity = stk->capacity;

    if (size + 1 > capacity)
    {
        *new_capacity = capacity * STACK_CAPACITY_MULTIPLIER;
    }

    else if ((ssize_t) (STACK_CAPACITY_MULTIPLIER * STACK_CAPACITY_MULTIPLIER * align_stack_size(size)) < capacity)
    {
        *new_capacity = capacity / STACK_CAPACITY_MULTIPLIER;
    }
    return NO_ERROR;
}

static stack_error_code realloc_stack(stack* stk, const ssize_t new_capacity)
{
    assert(stk);
    assert(new_capacity > stk->size);

    IF_SNITCH_ON(*(snitch_t*)(stk->data + stk->capacity) = 0);
    char*  realloc_ptr  = (char*)stk->data IF_SNITCH_ON(- sizeof(snitch_t));
    size_t realloc_size = new_capacity * sizeof(elem_t) IF_SNITCH_ON(+ 2 * sizeof(snitch_t));
    elem_t* new_data = (elem_t*) realloc(realloc_ptr, realloc_size);

    if (new_data)
    {
        elem_t* data_ptr = new_data IF_SNITCH_ON(+ sizeof(snitch_t) / sizeof(elem_t));

        fill_data_with_poison(data_ptr + stk->size, new_capacity - stk->size);
        stk->data     = data_ptr;
        stk->capacity = new_capacity;

        IF_SNITCH_ON(paste_snitch_value(stk->data + stk->capacity));
        IF_HASH_ON  (update_stack_hash(stk));
        LOG_STACK(stk);

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
        return 0 | 1 << realloc_stack(stk, new_capacity);

    IF_HASH_ON(update_stack_hash(stk));
    LOG_STACK(stk);

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
                return 0 | 1 << realloc_stack(stk, new_capacity);

            IF_HASH_ON(update_stack_hash(stk));
            LOG_STACK(stk);
        }
        return 0 | 1 << ANTI_OVERFLOW;
    }
    return 0 | 1 << NULL_VALUE_PTR;
}

stack_error_code destruct_stack(stack* stk)
{
    if (stk)
    {
        if (stk->data)
        {
            elem_t* data_ptr = (elem_t*)((char*)stk->data IF_SNITCH_ON(- sizeof(snitch_t)));

            if (stk->capacity > 0)
            {
                fill_data_with_poison(data_ptr, stk->capacity);
                stk->size = -1;
                stk->capacity = -1;

                #ifdef SNITCH
                stk->left_snitch = 0;
                stk->right_snitch = 0;
                #endif

                #ifdef HASH
                stk->struct_hash = 0;
                stk->data_hash = 0;
                #endif

                FREE_AND_NULL(data_ptr);

                stk = NULL;

                return NO_ERROR;
            }
            return NEGATIVE_CAPACITY;
        }
        return NULL_DATA;
    }
    return NULL_STACK_POINTER;
}

#ifdef SNITCH
static void paste_snitch_value(void* data_void)
{
    assert(data_void);
    snitch_t* data = (snitch_t*) data_void;
    data[0] = SNITCH_VALUE;
}
#endif

#ifdef HASH
static unsigned get_hash(void* key, size_t len)
{
    assert(key);
    assert(len > 0);

    const unsigned seed = 0xACAB1337;
    const unsigned MULTIPLY_VAL = 0xDED1DEAD;   // DED NOT DEAD
    const int ROTATE_VAL = 24;
    unsigned hash = (unsigned) (seed ^ len);

    unsigned char* data = (unsigned char *) key;

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

static unsigned get_stack_hash(stack* stk)
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

static void update_stack_hash(stack* stk)
{
    assert(stk);

    stk->struct_hash = get_stack_hash(stk);
    stk->data_hash   = get_hash(stk->data, stk->capacity * sizeof(elem_t));
}
#endif
