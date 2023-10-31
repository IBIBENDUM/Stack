#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// TODO: What is <direct.h>
// #include <direct.h>

#include <assert.h>
#include <time.h>

#include "stack.h"
#include "stack_logs.h"

#define TAB "    " // Because \t is too big

static bool get_log_file_name_with_folder(char* full_file_name);
static const char* get_log_file_name();
static void print_separator(FILE* file_ptr);

const size_t BYTE_ALIGN = 8;
const ssize_t FILE_NAME_SIZE = 128; ///< Max size for log file name
FILE* log_file_ptr = NULL;
const char* logs_folder_name = "logs";
const char* log_file_name = get_log_file_name(); ///< Set log_file_name to "stack_log_HH_MM_SS.html" format


static void print_separator(FILE* file_ptr)
{
    fprintf(file_ptr, "==================================================================\n");
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

stack_error_code (dump_stack)(FILE* file_ptr, stack* stk, unsigned error_bitmask, struct dump_info* info)
{
    assert(file_ptr);
    assert(info->file_name);
    assert(info->line);
    assert(info->func_name);

    print_separator(file_ptr);
    print_errors(file_ptr, error_bitmask);

    if(!stk)
        return NULL_STACK_POINTER;

    //---------------------with %p strange output
    fprintf(file_ptr, "stack[0x%llX] \"%s\" initialized from %s(%zd) %s\n", (long long unsigned int) stk, stk->init_info.var_name, stk->init_info.file_name,
                                                                                stk->init_info.line, stk->init_info.func_name);
    fprintf(file_ptr, "called from %s(%zd) %s\n", info->file_name, info->line, info->func_name);

    IF_SNITCH_ON(fprintf(file_ptr, "struct_left_snitch = 0x%llX\n", (long long unsigned int) stk->left_snitch));
    fprintf(file_ptr, "{\n" TAB "size = %zd\n" TAB "capacity = %zd\n" TAB "data[0x%llX]\n", stk->size, stk->capacity, (long long unsigned int) stk->data);

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

                for (ssize_t i = 0; i < stk->capacity; i++)
                {
                    while (i < stk->capacity && stk->data[i] != POISON_VALUE)
                    {
                        fprintf(file_ptr, TAB TAB "*[%zd] = " ELEM_FORMAT "\n", i, stk->data[i]);
                        i++;
                    }
                    if (i < stk->capacity && stk->data[i - 1] != POISON_VALUE)
                        fprintf(file_ptr, TAB TAB "... (POISON)\n");
                }
            }
            IF_SNITCH_ON(fprintf(file_ptr, TAB TAB "data_right_snitch = 0x%llX\n", *(snitch_t*)(stk->data + stk->capacity)));
            fprintf(file_ptr, TAB "}\n");
        }
    }
    fprintf(file_ptr, "}\n");
    IF_SNITCH_ON(fprintf(file_ptr, "struct_right_snitch = 0x%llX\n", stk->right_snitch));

    IF_HASH_ON(fprintf(file_ptr, "struct_hash = 0x%llX\n", (long long unsigned int) stk->struct_hash));
    IF_HASH_ON(fprintf(file_ptr, "data_hash = 0x%llX\n", (long long unsigned int) stk->data_hash));

    print_separator(file_ptr);
    if (!stk->data) return NULL_DATA;

    return NO_ERROR;
}


static const char* get_log_file_name()
{
    static char file_name[FILE_NAME_SIZE] = {};

    time_t current_time = time(0);
    struct tm* tm_info = localtime(&current_time);
    strftime(file_name, sizeof(file_name), "stack_log_%H_%M_%S.html", tm_info);

    return file_name;
}

bool get_log_file_name_with_folder(char* full_file_name)
{
    if (!strcat(full_file_name, logs_folder_name))
        return true;
    if (!strcat(full_file_name, "/"))
        return true;
    if (!strcat(full_file_name, log_file_name))
        return true;

    return false;
}

bool open_log_file()
{
    if (!log_file_name)
    {
        DEBUG_MSG("Incorrect logs file name");
        return true;
    }
    // if (_mkdir(logs_folder_name) == ENOENT)
    // {
    //     DEBUG_MSG("Path not found");
    //     return true;
    // }
    char full_file_name[FILE_NAME_SIZE] = {};
    if (get_log_file_name_with_folder(full_file_name))
    {
        DEBUG_MSG("Can't get full file name");
        return true;
    }

    log_file_ptr = fopen(full_file_name, "a");
    if (!log_file_ptr)
    {
        DEBUG_MSG("[%s] %s: Error at file open\n", __FILE__, __PRETTY_FUNCTION__);
        return true;
    }
    fprintf(log_file_ptr, "<!DOCTYPE html>\n");
    fprintf(log_file_ptr, "<html>\n<body>\n<pre>\n");

    return false;
}

void log_stack_to_file(stack* stk)
{
    if (!log_file_ptr)
        open_log_file();

    unsigned error_bitmask = validate_stack(stk);
    if (error_bitmask)
    {
        fprintf(log_file_ptr, "<font color = #FF0000 size = 4>\n");
        dump_stack(log_file_ptr, stk, error_bitmask);
        fprintf(log_file_ptr, "</font>\n");
        close_log_file();
    }
    else
    {
        fprintf(log_file_ptr, "<font color = #BBBBBB size = 1>\n");
        dump_stack(log_file_ptr, stk, error_bitmask);
        fprintf(log_file_ptr, "</font>\n");
    }
}

bool close_log_file()
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
