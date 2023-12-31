#ifndef STACK_H
#define STACK_H

#include <limits.h>

/// -DDEBUG  enable debug messages and logs
/// -DHASH   enable hash protection
/// -DSNITCH enable shitch protection

#ifdef SNITCH
    #define IF_SNITCH_ON(...) __VA_ARGS__
    #define IF_SNITCH_OFF(...)
#else
    #define IF_SNITCH_ON(...)
    #define IF_SNITCH_OFF(...) __VA_ARGS__
#endif

#ifdef HASH
    #define IF_HASH_ON(...) __VA_ARGS__
    #define IF_HASH_OFF(...)
#else
    #define IF_HASH_ON(...)
    #define IF_HASH_OFF(...) __VA_ARGS__
#endif

// TODO: move to other file
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

/// Macro wraps to enum
/// {
///     NO_ERROR,
///     NULL_STACK_POINTER,
///     ...
///     ANTI_OVERFLOW,
/// }
enum stack_error_code
{
    #define INIT_ERROR(ERR_CODE) ERR_CODE,
        STACK_ERRORS

    // TODO: It's better to use something like "stack.inc" in a separate instead
    #undef INIT_ERROR
};

// TODO: Just make your error_codes the same
//       as your error values...


struct dump_info // TODO: better name
{
    const char* file_name;
    const size_t line;
    const char* func_name;
};

struct initialize_info // TODO: better name
{
    const char* var_name;

    // TODO: this looks like your dump_info:
    const char* file_name;
    ssize_t line;
    const char* func_name;
};

// TODO: align "\"
#define init_stack(STK)\
    do {\
        struct initialize_info INFO = { .var_name = #STK,\
                                        .file_name = __FILE__,\
                                        .line = __LINE__,\
                                        .func_name = __PRETTY_FUNCTION__\
                                      };\
        init_stack(&STK, &INFO);\
    } while(0)

// ==> Meaning... Do this:

/* #define init_stack(STK)                                                  \ */
/*     do {                                                                 \ */
/*         struct initialize_info INFO = { .var_name = #STK,                \ */
/*                                         .file_name = __FILE__,           \ */
/*                                         .line = __LINE__,                \ */
/*                                         .func_name = __PRETTY_FUNCTION__ \ */
/*                                       };                                 \ */
/*         init_stack(&STK, &INFO);                                         \ */
/*     } while(0) */


// ==> Instead of this:

/* #define init_stack(STK)\ */
/*     do {\ */
/*         struct initialize_info INFO = { .var_name = #STK,\ */
/*                                         .file_name = __FILE__,\ */
/*                                         .line = __LINE__,\ */
/*                                         .func_name = __PRETTY_FUNCTION__\ */
/*                                       };\ */
/*         init_stack(&STK, &INFO);\ */
/*     } while(0) */


// TODO: define AND typedef?
#define VALUE_TYPE int
#define ELEM_FORMAT "%d"


// TODO: You can achive generic stack template parametrization
//       via a macros and ##type mangling
const VALUE_TYPE POISON_VALUE = INT_MAX;
typedef VALUE_TYPE elem_t;
typedef unsigned long long snitch_t;
const snitch_t SNITCH_VALUE = 0xABADC0DEDA551337;     // A BAD CODED ASS 1337

typedef struct stack
{
    IF_SNITCH_ON(snitch_t left_snitch = SNITCH_VALUE;)

    struct initialize_info init_info;
    elem_t* data;
    ssize_t size;
    ssize_t capacity;

    #ifdef HASH
    unsigned struct_hash;
    unsigned data_hash;
    #endif

    IF_SNITCH_ON(snitch_t right_snitch = SNITCH_VALUE;)
} stack;

/// @brief Initialize stack with poison value@n
///        Should be used through macro init_stack
stack_error_code (init_stack)(stack* stk, struct initialize_info* info);
// TODO:         ^          ^ whyyy?

/// @brief Write value to stack to last position
/// @return Bitmask that contains errors
unsigned push_stack(stack* stk, const elem_t value);
// TODO:                        ^~~~~ why const here?

/// @brief Get last value from stack
/// @return Bitmask that contains errors
unsigned pop_stack(stack* stk, elem_t* const value);

/// @brief Destruct stack and its members
stack_error_code destruct_stack(stack* stk);

unsigned validate_stack(stack* stk);

#endif
