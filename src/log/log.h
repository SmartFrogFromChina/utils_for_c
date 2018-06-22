#ifndef __LOG__H_
#define __LOG__H_

#include <stdio.h>
#include <stdarg.h>

#define ENABLE_LOG
#define LOG_USE_COLOR
//#define LOG_USE_SECOND
void log_log(int level, const char *file,const char *function, int line, const char *fmt, ...);

enum { 
    LOGGER_TRACE, 
    LOGGER_DEBUG, 
    LOGGER_INFO, 
    LOGGER_WARN, 
    LOGGER_ERROR, 
    LOGGER_FATAL 
};


#ifdef ENABLE_LOG
#define trace(...) log_log(LOGGER_TRACE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define debug(...) log_log(LOGGER_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define info(...)  log_log(LOGGER_INFO,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define warn(...)  log_log(LOGGER_WARN,  __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define error(...) log_log(LOGGER_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define fatal(...) log_log(LOGGER_FATAL, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define trace(...) 
#define debug(...) 
#define info(...)  
#define warn(...)  
#define error(...) 
#define fatal(...) 

#endif

//必须调用：log初始化
int log_init(int level,int quiet);
//必须调用：log清理动作
void log_deinit();

//可选调用：将log信息记录到文件中的初始化动作
int file_log_init(char *file_name);
//可选调用：将log信息记录到文件中的清理动作
void file_log_deinit();

#endif
