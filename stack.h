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
const char* get_log_file_name();

FILE* log_file_ptr = NULL;
const char* log_file_name = get_log_file_name();

#ifdef DEBUG
    #define DEBUG_MSG(FORMAT, ...)\
        do {\
            printf(FORMAT, ##__VA_ARGS__); /* BAH: "##" because of empty __VA_ARGS__*/ \
        } while (0)

    #define dump_stack(FILE_PTR, STK)\
        do {\
            struct dump_info INFO = { .file_name = __FILE__, \
                                     .line = __LINE__,\
                                     .func_name = __PRETTY_FUNCTION__\
                                    };\
            (dump_stack)(FILE_PTR, STK, &INFO);\
        } while(0)
#else
    #define DEBUG_MSG(FORMAT, ...)
    #define dump_stack(FILE_PTR, STK)
#endif

#define FREE_AND_NULL(PTR)\
    do {\
        free(PTR);\
        PTR = NULL;\
    } while(0)

#define RETURN_ERR_IF_STK_WRONG(PTR)\
    do {\
        unsigned ERROR_BITMASK = validate_stack(PTR);\
        if (ERROR_BITMASK)\
        {\
            DEBUG_MSG("Error occurred:\n");\
            print_errors(ERROR_BITMASK);\
            dump_stack(stdout, PTR);\
            assert(0);\
            return ERROR_BITMASK;\
        }\
    } while(0)

const ssize_t DEFAULT_STACK_SIZE = 1;
#define init_stack(STK)\
    do {\
        struct initialize_info INFO = { .var_name = #STK,\
                                        .file_name = __FILE__,\
                                        .line = __LINE__,\
                                        .func_name = __PRETTY_FUNCTION__\
                                      };\
        (init_stack)(&STK, &INFO);\
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

#if !defined(VALUE_TYPE) || !defined(POISON_VAL) || !defined(ELEM_FORMAT)
    #undef VALUE_TYPE
    #undef POISON_VAL
    #undef ELEM_FORMAT

    #define VALUE_TYPE int
    #define ELEM_FORMAT "%d"
    #define POISON_VAL INT_MAX
#endif

const VALUE_TYPE POISON_VALUE = POISON_VAL;
typedef VALUE_TYPE elem_t;
typedef unsigned long long snitch_t;
const snitch_t SNITCH_VALUE = 0xABADC0DEDA551337;     // A BAD CODED ASS 1337

// Should i make if here?
#ifdef HASH
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
#endif

typedef struct STACK
{
    #ifdef SNITCH
    snitch_t left_snitch = SNITCH_VALUE;
    #endif

    struct initialize_info init_info;
    elem_t* data;
    ssize_t size;
    ssize_t capacity;

    #ifdef HASH
    unsigned struct_hash;
    unsigned data_hash;
    #endif

    #ifdef SNITCH
    snitch_t right_snitch = SNITCH_VALUE;
    #endif
} stack;

static void print_errors(const unsigned error_bitmask)
{
    for (size_t i = 0; i < sizeof(error_bitmask) * 8; i++)
    {
        if (error_bitmask & (1 << i))
        {
            switch (i)
            {
                #define INIT_ERROR(ERR_NAME)\
                    case ERR_NAME: DEBUG_MSG("%s\n", #ERR_NAME); break;
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

    if (!stk)                                                        error_bitmask |= 1 << NULL_STACK_POINTER;
    if (!stk->data)                                                  error_bitmask |= 1 << NULL_DATA;

    #ifdef SNITCH
    if (stk->left_snitch  != SNITCH_VALUE)                           error_bitmask |= 1 << DEAD_LEFT_SNITCH;
    if (stk->right_snitch != SNITCH_VALUE)                           error_bitmask |= 1 << DEAD_RIGHT_SNITCH;
    #endif

    if (stk->capacity     < 0)                                       error_bitmask |= 1 << NEGATIVE_CAPACITY;
    if (stk->size         < 0)                                       error_bitmask |= 1 << NEGATIVE_SIZE;
    if (stk->size         > stk->capacity)                           error_bitmask |= 1 << WRONG_SIZE;

    if (stk) // check hash only if other checks went correct
    {
        #ifdef SNITCH
        if (*((snitch_t*) stk->data - 1)
            != SNITCH_VALUE)                                             error_bitmask |= 1 << DEAD_LEFT_DATA_SNITCH;
        if (*(snitch_t*)(stk->data + stk->capacity)
            != SNITCH_VALUE)                                             error_bitmask |= 1 << DEAD_RIGHT_DATA_SNITCH;
        #endif

        #ifdef HASH
            if (stk->struct_hash  != get_stack_hash(stk))                error_bitmask |= 1 << WRONG_STRUCT_HASH;
            if (stk->data_hash    != get_hash(stk->data,
                                stk->capacity * sizeof(elem_t)))       error_bitmask |= 1 << WRONG_DATA_HASH;
        #endif
    }

    return error_bitmask;
}

void print_only_to_log(FILE* file_ptr, const char* string)
{
    if (file_ptr == log_file_ptr)
    {
        fprintf(file_ptr, string);
    }
}

void print_only_to_console(FILE* file_ptr, const char* string)
{
    if (file_ptr == stdout)
    {
        fprintf(file_ptr, string);
    }
}

void print_newline_char(FILE* file_ptr)
{
    print_only_to_log(file_ptr, "</br>");
    putc('\n', file_ptr);
}

void print_tab_char(FILE* file_ptr)
{
    print_only_to_log(file_ptr, "&emsp;");
    print_only_to_console(file_ptr, TAB);
}

static void print_separator(FILE* file_ptr)
{
    print_only_to_log(file_ptr, TAB);
    print_newline_char(file_ptr);
    print_only_to_log(file_ptr, TAB);
    fprintf(file_ptr, "==================================================================");
    print_newline_char(file_ptr);
    print_only_to_log(file_ptr, TAB);
    print_newline_char(file_ptr);
}

#include <stdarg.h>

void fprintf_tab(FILE* file_ptr, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    print_only_to_log(file_ptr, TAB);
    vfprintf(file_ptr, format, args);
    va_end(args);
}

stack_error_code (dump_stack)(FILE* file_ptr, stack* stk, struct dump_info* info)
{
    assert(file_ptr);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    print_separator(file_ptr);

    fprintf_tab(file_ptr, "stack[0x%llX] \"%s\" initialized from %s(%d) %s", stk, stk->init_info.var_name, stk->init_info.file_name,
                                                                              stk->init_info.line, stk->init_info.func_name);
    print_newline_char(file_ptr);
    fprintf_tab(file_ptr, "called from %s(%d) %s", info->file_name, info->line, info->func_name);
    print_newline_char(file_ptr);

    if(!stk)
        return NULL_STACK_POINTER;

    #ifdef SNITCH
    fprintf_tab(file_ptr, "struct_left_snitch = 0x%llX", stk->left_snitch);
    print_newline_char(file_ptr);
    #endif

    fprintf_tab(file_ptr, "{");
    print_newline_char(file_ptr);
    print_only_to_log(file_ptr, TAB);
    print_tab_char(file_ptr);
    fprintf(file_ptr, "size = %d", stk->size);
    print_newline_char(file_ptr);
    print_only_to_log(file_ptr, TAB);
    print_tab_char(file_ptr);
    fprintf(file_ptr, "capacity = %d", stk->capacity);
    print_newline_char(file_ptr);
    print_only_to_log(file_ptr, TAB);
    print_tab_char(file_ptr);
    fprintf(file_ptr, "data[0x%llX]", stk->data);
    print_newline_char(file_ptr);

    if (stk->data)
    {
        print_only_to_log(file_ptr, TAB);
        print_tab_char(file_ptr);
        fprintf(file_ptr, "{");
        print_newline_char(file_ptr);
        #ifdef SNITCH
        print_only_to_log(file_ptr, TAB);
        print_tab_char(file_ptr);
        print_tab_char(file_ptr);
        fprintf(file_ptr, "data_left_snitch = 0x%llX", *(snitch_t*)((char*)stk->data - sizeof(long long)));
        print_newline_char(file_ptr);
        #endif

        if (stk->capacity < 1)
        {
            print_only_to_log(file_ptr, TAB);
            print_tab_char(file_ptr);
            print_tab_char(file_ptr);
            fprintf(file_ptr, "NULL");
            print_newline_char(file_ptr);
        }

        else
        {

            for (size_t i = 0; i < stk->capacity; i++)
            {
                while (i < stk->capacity && stk->data[i] != POISON_VALUE)
                {
                    print_only_to_log(file_ptr, TAB);
                    print_tab_char(file_ptr);
                    print_tab_char(file_ptr);
                    fprintf(file_ptr, "*[%d] = " ELEM_FORMAT, i, stk->data[i]);
                    print_newline_char(file_ptr);
                    i++;
                }
                if (i < stk->capacity && stk->data[i - 1] != POISON_VALUE)
                {
                    print_only_to_log(file_ptr, TAB);
                    print_tab_char(file_ptr);
                    print_tab_char(file_ptr);
                    fprintf(file_ptr, "... (POISON)");
                    print_newline_char(file_ptr);
                }
            }
        }
        #ifdef SNITCH
        print_only_to_log(file_ptr, TAB);
        print_tab_char(file_ptr);
        print_tab_char(file_ptr);
        fprintf(file_ptr, "data_right_snitch = 0x%llX", *(snitch_t*)(stk->data + stk->capacity));
        print_newline_char(file_ptr);
        #endif

        print_only_to_log(file_ptr, TAB);
        print_tab_char(file_ptr);
        fprintf(file_ptr, "}");
        print_newline_char(file_ptr);
    }

    fprintf_tab(file_ptr, "}");
    print_newline_char(file_ptr);
    #ifdef SNITCH
    fprintf_tab(file_ptr, "struct_right_snitch = 0x%llX", stk->right_snitch);
    print_newline_char(file_ptr);
    #endif

    #ifdef HASH
    fprintf_tab(file_ptr, "struct_hash = 0x%llX", stk->struct_hash);
    print_newline_char(file_ptr);
    fprintf_tab(file_ptr, "data_hash = 0x%llX", stk->data_hash);
    print_newline_char(file_ptr);
    #endif

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
        *new_capacity = capacity * 2;
    }

    else if (4 * align_stack_size(size) < capacity)
    {
        *new_capacity = capacity / 2;
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

    // Realloc vs calloc + memcpy?
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

        #ifdef SNITCH
        paste_snitch_value(stk->data + stk->capacity);
        #endif

        #ifdef HASH
        update_stack_hash(stk);
        #endif

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

    #ifdef HASH
    update_stack_hash(stk);
    #endif

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
            #ifdef HASH
            update_stack_hash(stk);
            #endif
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
        ssize_t capacity = align_stack_size(1);
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
            stk->data_hash = get_hash(stk->data, capacity * sizeof(elem_t));
            #endif
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

                #ifdef SNITCH
                stk->left_snitch = 0;
                stk->right_snitch = 0;
                #endif

                #ifdef HASH
                stk->struct_hash = 0;
                stk->data_hash = 0;
                #endif

                FREE_AND_NULL(data_ptr);
                return NO_ERROR;
            }
            return NEGATIVE_CAPACITY;
        }
        return NULL_DATA;
    }
    return NULL_STACK_POINTER;
}

const char* get_log_file_name()
{
    static char file_name[128] = {};

    time_t current_time = time(0);
    struct tm* tm_info = localtime(&current_time);

    strftime(file_name, sizeof(file_name), "logs/stack_log_%H_%M_%S.html", tm_info);
    return file_name;
}

void log_stack_to_html(stack* stk)
{
    printf("stk_ptr = %p\n", stk);
    unsigned error_bitmask = validate_stack(stk);
    printf("stk_ptr = %p\n", stk);
    if (error_bitmask)
    {
        fprintf(log_file_ptr, "\t<font color = #FF0000 size = 4>\n");
        dump_stack(log_file_ptr, stk);
        fprintf(log_file_ptr, "\t</font>\n");
    }
    else
    {
        fprintf(log_file_ptr, "\t<font color = #BBBBBB size = 1>\n");
        dump_stack(log_file_ptr, stk);
        fprintf(log_file_ptr, "\t</font>\n");
    }
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
    fprintf(log_file_ptr, "<html>\n<body>\n");

    return false;
}


bool close_html()
{
    if (!log_file_ptr)
        return true;

    fprintf(log_file_ptr, "</body>\n</html>\n");

    if (fclose(log_file_ptr))
    {
        DEBUG_MSG("[%s] %s: Error at file closing\n", __FILE__, __PRETTY_FUNCTION__);
        return true;
    }
    log_file_ptr = NULL;

    return false;
}

#endif
