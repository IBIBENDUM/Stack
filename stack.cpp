#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "stack.h"

#define TAB "    " // Because \t is too big

const size_t BYTE_ALIGN = 8;
const size_t STACK_CAPACITY_MULTIPLIER = 2;
const ssize_t DEFAULT_STACK_SIZE = 1;
const ssize_t FILE_NAME_SIZE = 128;

static const char* get_log_file_name();

FILE* log_file_ptr = NULL;
const char* log_file_name = get_log_file_name();

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
            assert(0);\
            return ERROR_BITMASK;\
        }\
    } while(0)

#define write_stack_log(STK)\
    do {\
        unsigned ERROR_BITMASK = validate_stack(STK);\
        struct dump_info INFO = {\
                                    .file_name = __FILE__,\
                                    .line = __LINE__,\
                                    .func_name = __PRETTY_FUNCTION__\
                                };\
        (write_stack_log)(STK, ERROR_BITMASK, &INFO);\
    } while(0)

static unsigned get_hash(const void* key, size_t len)
{
    assert(key);
    assert(len > 0);

    const unsigned seed = 0xACAB1337;
    const unsigned MULTIPLY_VAL = 0xDED1DEAD;   // DED NOT DEAD
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

static void print_errors(FILE* file_ptr, const unsigned error_bitmask)
{
    for (size_t i = 0; i < sizeof(error_bitmask) * BYTE_ALIGN; i++)
    {
        if (error_bitmask & (1 << i))
        {
            switch (i)
            {
                #define INIT_ERROR(ERR_NAME)\
                    case ERR_NAME: fprintf(file_ptr, "%s\n", #ERR_NAME); break;
                    STACK_ERRORS
                #undef INIT_ERROR
                #undef STACK_ERRORS
                default: break;
            }
        }
    }
}

#ifdef HASH
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
#endif

static unsigned validate_stack(stack* stk)
{
    unsigned error_bitmask = 0;

    if (!stk)
    {
        error_bitmask |= 1 << NULL_STACK_POINTER;
        return error_bitmask;
    }

    if (stk->capacity     < 0)
        error_bitmask |= 1 << NEGATIVE_CAPACITY;

    if (stk->size         < 0)
        error_bitmask |= 1 << NEGATIVE_SIZE;

    if (stk->size         > stk->capacity)
        error_bitmask |= 1 << WRONG_SIZE;

    if (!stk->data)
        error_bitmask |= 1 << NULL_DATA;

    IF_SNITCH_ON
    (
        if (stk->left_snitch  != SNITCH_VALUE)
            error_bitmask |= 1 << DEAD_LEFT_SNITCH;

        if (stk->right_snitch != SNITCH_VALUE)
            error_bitmask |= 1 << DEAD_RIGHT_SNITCH;
    )

    IF_HASH_ON
    (
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
    )

    IF_SNITCH_ON
    (
        if (!stk->data)
        {
            if (*((snitch_t*) stk->data - 1) != SNITCH_VALUE)
                error_bitmask |= 1 << DEAD_LEFT_DATA_SNITCH;

            if (*(snitch_t*)(stk->data + stk->capacity) != SNITCH_VALUE)
                error_bitmask |= 1 << DEAD_RIGHT_DATA_SNITCH;
        }
    )
    return error_bitmask;
}

static void print_separator(FILE* file_ptr)
{
    fprintf(file_ptr, "==================================================================\n");
}

stack_error_code (dump_stack)(FILE* file_ptr, stack* stk, unsigned error_bitmask, struct dump_info* info)
{
    assert(file_ptr);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    print_separator(file_ptr);
    print_errors(file_ptr, error_bitmask);
    fprintf(file_ptr, "stack[0x%llX] \"%s\" initialized from %s(%d) %s\n", stk, stk->init_info.var_name, stk->init_info.file_name,
                                                                              stk->init_info.line, stk->init_info.func_name);
    fprintf(file_ptr, "called from %s(%d) %s\n", info->file_name, info->line, info->func_name);

    if(!stk)
    {
        fprintf(file_ptr, "NULL_STACK_POINTER\n");
        return NULL_STACK_POINTER;
    }
    IF_SNITCH_ON(fprintf(file_ptr, "struct_left_snitch = 0x%llX\n", stk->left_snitch);)

    fprintf(file_ptr, "{\n" TAB "size = %d\n" TAB "capacity = %d\n" TAB "data[0x%llX]\n", stk->size, stk->capacity, stk->data);
    if (!(error_bitmask & (0 | 1 << WRONG_STRUCT_HASH)))
    {
        if (stk->data)
        {
            fprintf(file_ptr, TAB "{\n");
            IF_SNITCH_ON(fprintf(file_ptr, TAB TAB "data_left_snitch = 0x%llX\n", *(snitch_t*)((char*)stk->data - sizeof(long long))));

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
            IF_SNITCH_ON(fprintf(file_ptr, TAB TAB "data_right_snitch = 0x%llX\n", *(snitch_t*)(stk->data + stk->capacity));)
            fprintf(file_ptr, TAB "}\n");
        }
    }
    fprintf(file_ptr, "}\n");
    IF_SNITCH_ON(fprintf(file_ptr, "struct_right_snitch = 0x%llX\n", stk->right_snitch);)

    IF_HASH_ON
    (
    fprintf(file_ptr, "struct_hash = 0x%llX\n", stk->struct_hash);
    fprintf(file_ptr, "data_hash = 0x%llX\n", stk->data_hash);
    )

    print_separator(file_ptr);
    if (!stk->data) return NULL_DATA;
    return NO_ERROR;
}

#ifdef SNITCH
static void paste_snitch_value(void* data_void)
{
    assert(data_void);
    snitch_t* data = (snitch_t*) data_void;
    data[0] = SNITCH_VALUE;
}
#endif

static ssize_t align_stack_size(ssize_t size)
{
    return (sizeof(elem_t) * size + BYTE_ALIGN - sizeof(elem_t) * size % BYTE_ALIGN) / sizeof(elem_t);
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

    else if (STACK_CAPACITY_MULTIPLIER * STACK_CAPACITY_MULTIPLIER * align_stack_size(size) < capacity)
    {
        *new_capacity = capacity / STACK_CAPACITY_MULTIPLIER;
    }

    return NO_ERROR;
}

static stack_error_code fill_data_with_poison(elem_t* data_ptr, size_t size)
{
    assert(data_ptr);
    assert(size >= 0);

    for (size_t i = 0; i < size; i++)
    {
        assert(0 <= i && i < size);

        data_ptr[i] = POISON_VALUE;
    }

    return NO_ERROR;
}

#ifdef HASH
static void update_stack_hash(stack* stk)
{
    assert(stk);

    stk->struct_hash = get_stack_hash(stk);
    stk->data_hash   = get_hash(stk->data, stk->capacity * sizeof(elem_t));
}
#endif

static stack_error_code realloc_stack(stack* stk, const ssize_t new_capacity)
{
    assert(stk);
    assert(new_capacity > stk->size);

    #ifdef SNITCH
    *(snitch_t*)(stk->data + stk->capacity) = 0;
    elem_t* new_data = (elem_t*) realloc((char*)stk->data - sizeof(snitch_t), new_capacity * sizeof(new_data[0]) + 2 * sizeof(snitch_t));
    #else
    elem_t* new_data = (elem_t*) realloc((char*)stk->data, new_capacity * sizeof(new_data[0]));
    #endif

    if (new_data)
    {
        #ifdef SNITCH
        elem_t* data_ptr = new_data + sizeof(snitch_t) / sizeof(new_data[0]);
        #else
        elem_t* data_ptr = new_data;
        #endif

        fill_data_with_poison(data_ptr + stk->size, new_capacity - stk->size);
        stk->data = data_ptr;
        stk->capacity = new_capacity;

        IF_SNITCH_ON(paste_snitch_value(stk->data + stk->capacity);)
        IF_HASH_ON(update_stack_hash(stk);)

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
    IF_HASH_ON(update_stack_hash(stk);)

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
            IF_HASH_ON(update_stack_hash(stk);)
        }
        return 0 | 1 << ANTI_OVERFLOW;
    }
    return 0 | 1 << NULL_VALUE_PTR;
}

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
        #ifdef SNITCH
        const size_t data_size = capacity + 2 * sizeof(snitch_t) / sizeof(elem_t);
        elem_t* data = (elem_t*) calloc(data_size, sizeof(data[0]));
        #else
        elem_t* data = (elem_t*) calloc(capacity, sizeof(data[0]));
        #endif

        if (data)
        {
            #ifdef SNITCH
            stk->data     = (elem_t*) ((char*)data + sizeof(snitch_t));
            #else
            stk->data     = data;
            #endif

            stk->size     = 0;
            stk->capacity = capacity;

            IF_SNITCH_ON
            (
            paste_snitch_value(data);
            paste_snitch_value(stk->data + capacity);
            )

            fill_data_with_poison(stk->data, stk->capacity);

            stk->init_info.var_name  = info->var_name;
            stk->init_info.file_name = info->file_name;
            stk->init_info.line      = info->line;
            stk->init_info.func_name = info->func_name;

            IF_HASH_ON
            (
            stk->struct_hash = get_stack_hash(stk);
            stk->data_hash = get_hash(stk->data, capacity * sizeof(elem_t));
            )
        }
        return NULL_DATA;
    }
    return NULL_STACK_POINTER;
}

stack_error_code destruct_stack(stack* stk)
{
    if (stk)
    {
        if (stk->data)
        {
            #ifdef SNITCH
            elem_t* data_ptr = (elem_t*)((char*)stk->data - sizeof(snitch_t));
            #else
            elem_t* data_ptr = stk->data;
            #endif

            if (stk->capacity > 0)
            {
                fill_data_with_poison(data_ptr, stk->capacity);
                stk->size = -1;
                stk->capacity = -1;

                IF_SNITCH_ON
                (
                stk->left_snitch = 0;
                stk->right_snitch = 0;
                )

                IF_HASH_ON
                (
                stk->struct_hash = 0;
                stk->data_hash = 0;
                )

                FREE_AND_NULL(data_ptr);
                return NO_ERROR;
            }
            return NEGATIVE_CAPACITY;
        }
        return NULL_DATA;
    }
    return NULL_STACK_POINTER;
}

static const char* get_log_file_name()
{
    static char file_name[FILE_NAME_SIZE] = {};

    time_t current_time = time(0);
    struct tm* tm_info = localtime(&current_time);

    strftime(file_name, sizeof(file_name), "logs/stack_log_%H_%M_%S.html", tm_info);
    return file_name;
}

bool open_html()
{
    if (!log_file_name)
        return true;

    log_file_ptr = fopen(log_file_name, "a");
    if (!log_file_ptr)
    {
        DEBUG_MSG("[%s] %s: Error at file open\n", __FILE__, __PRETTY_FUNCTION__);
        return true;
    }
    fprintf(log_file_ptr, "<!DOCTYPE html>\n");
    fprintf(log_file_ptr, "<html>\n<body>\n<pre>\n");

    return false;
}

void log_stack_to_html(stack* stk)
{
    if (!log_file_ptr)
        open_html();

    unsigned error_bitmask = validate_stack(stk);
    if (error_bitmask)
    {
        fprintf(log_file_ptr, "<font color = #FF0000 size = 4>\n");
        dump_stack(log_file_ptr, stk, 0);
        fprintf(log_file_ptr, "</font>\n");
        close_html();
    }
    else
    {
        fprintf(log_file_ptr, "<font color = #BBBBBB size = 1>\n");
        dump_stack(log_file_ptr, stk, 0);
        fprintf(log_file_ptr, "</font>\n");
    }
}

bool close_html()
{
    if (!log_file_ptr)
        return true;

    fprintf(log_file_ptr, "</pre>\n</body>\n</html>\n");

    if (fclose(log_file_ptr))
    {
        DEBUG_MSG("[%s] %s: Error at file closing\n", __FILE__, __PRETTY_FUNCTION__);
        return true;
    }
    log_file_ptr = NULL;

    return false;
}
