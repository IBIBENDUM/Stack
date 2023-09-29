#ifndef STACK_H
#define STACK_H

#include <limits.h>

#if !defined(VALUE_TYPE) || !defined(POISON_VAL) || !defined(ELEM_FORMAT)
    #undef VALUE_TYPE
    #undef POISON_VAL
    #undef ELEM_FORMAT

    #define VALUE_TYPE int
    #define ELEM_FORMAT "%d"
    #define POISON_VAL INT_MAX
#endif

#define init_stack(STK)\
    do {\
        struct initialize_info INFO = { .var_name = #STK,\
                                        .file_name = __FILE__,\
                                        .line = __LINE__,\
                                        .func_name = __PRETTY_FUNCTION__\
                                      };\
        (init_stack)(&STK, &INFO);\
    } while(0)

#ifdef DEBUG
    #define DEBUG_MSG(FORMAT, ...)\
        do {\
            printf(FORMAT, ##__VA_ARGS__); /* BAH: "##" because of empty __VA_ARGS__*/ \
        } while (0)

    #define dump_stack(FILE_PTR, STK, ERROR)\
        do {\
            struct dump_info INFO = { .file_name = __FILE__, \
                                     .line = __LINE__,\
                                     .func_name = __PRETTY_FUNCTION__\
                                    };\
            (dump_stack)(FILE_PTR, STK, ERROR, &INFO);\
        } while(0)
#else
    #define DEBUG_MSG(FORMAT, ...)
    #define dump_stack(FILE_PTR, STK, ERROR)
#endif

#ifdef SNITCH
    #define IF_SNITCH_ON(...) __VA_ARGS__
#else
    #define IF_SNITCH_ON(...)
#endif

#ifdef HASH
    #define IF_HASH_ON(...) __VA_ARGS__
#else
    #define IF_HASH_ON(...)
#endif

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

const VALUE_TYPE POISON_VALUE = POISON_VAL;
typedef VALUE_TYPE elem_t;
typedef unsigned long long snitch_t;
const snitch_t SNITCH_VALUE = 0xABADC0DEDA551337;     // A BAD CODED ASS 1337

typedef struct STACK
{
    IF_SNITCH_ON
    (
    snitch_t left_snitch = SNITCH_VALUE;
    )

    struct initialize_info init_info;
    elem_t* data;
    ssize_t size;
    ssize_t capacity;

    IF_HASH_ON
    (
    unsigned struct_hash;
    unsigned data_hash;
    )

    IF_SNITCH_ON
    (
    snitch_t right_snitch = SNITCH_VALUE;
    )
} stack;

stack_error_code (init_stack)(stack* stk, struct initialize_info* info);

unsigned push_stack(stack* stk, const elem_t value);

unsigned pop_stack(stack* stk, elem_t* const value);

stack_error_code destruct_stack(stack* stk);

stack_error_code (dump_stack)(FILE* file_ptr, stack* stk, unsigned error_bitmask, struct dump_info* info);

bool open_html();

void log_stack_to_html(stack* stk);

bool close_html();

#endif
