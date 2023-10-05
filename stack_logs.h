#ifndef STACK_LOGS_H
#define STACK_LOGS_H

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
            dump_stack(FILE_PTR, STK, ERROR, &INFO);\
        } while(0)
#else
    #define DEBUG_MSG(FORMAT, ...)
    #define dump_stack(FILE_PTR, STK, ERROR)
#endif

extern const char* log_file_name;  ///< File name for log functions
extern const char* logs_folder_name;

/// @brief Print all info about stack that can safely get
stack_error_code (dump_stack)(FILE* file_ptr, stack* stk, unsigned error_bitmask, struct dump_info* info);

/// @brief Open html file for log@n
/// @return True if error occurred
bool open_log_file();

/// @brief Print dump_stack to log file
void log_stack_to_file(stack* stk);

/// @brief Close html file
/// @return True if error occurred
bool close_log_file();

#endif
