#ifndef __PRETTY_LOG_H__
#define __PRETTY_LOG_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>


enum {
    LOGGER_FATAL,
    LOGGER_ERROR,
    LOGGER_WARN,
    LOGGER_INFO,
    LOGGER_DEBUG,
    LOGGER_RAW,
    MAX_TYPE
};



void get_current_time(char *ts,int size);
int pretty_log_init(int argc,char **argv);



#define MAX_SIZE    64

#define TELNET   "/dev/pts/0"
#define CONSOLE  "/dev/console"
#define LOG_OUT_PATH CONSOLE

extern char log_level_str[MAX_TYPE+1];
extern char log_file_path[MAX_SIZE];


void log_log(
    char level,        //log 类型（级别）：
    char lock,         //是否启用互斥锁
    char *cmd_str,     //log 开关控制
    char *path,         //log 输出文件路径
    char *time,         //时间字符串
    const char *file,   //调用该函数的文件名
    const char *func,   //函数名称
    const int line,     //调用该函数的行号
    char *fmt,          //用于输出的格式化字符串
    ...);

void raw_log(
    char level,
    char lock,
    char *cmd_str,
    const char *path,
    const char *fmt,
    ...);

#define _log(level,lock,x...) ({   \
    char ts[32]="";                 \
    get_current_time(ts,sizeof(ts));\
    log_log(level,lock,log_level_str,log_file_path,ts,__FILE__,__FUNCTION__,__LINE__,x);\
})

#define console_log(fmt,args...) ({         \
    FILE *fp = fopen(LOG_OUT_PATH, "a");    \
    if(!fp)                                 \
        fp=stdout;                          \
    fprintf(fp,"[%s:%s:%d]:",__FILE__,__FUNCTION__,__LINE__); \
    fprintf(fp,fmt,##args);                 \
    fprintf(fp,"\n");                       \
    if(fp!=stdout)                          \
        fclose(fp);                         \
})


#define log_nt(level,lock,x...)  log_log(level,lock,log_level_str,log_file_path,"",__FILE__,__FUNCTION__,__LINE__,x);
#define _rlog(level,lock,x...)   raw_log(level,lock,log_level_str,log_file_path,x);


//这一组宏 带锁、时间、颜色，适用于多线程程序调试，但不能用于信号回调函数中，以防死锁
#define raw(x...)       _rlog(LOGGER_RAW, 1,x);
#define debug(x...)     _log(LOGGER_DEBUG,1,x)
#define info(x...)      _log(LOGGER_INFO, 1,x)
#define warn(x...)      _log(LOGGER_WARN, 1,x)
#define error(x...)     _log(LOGGER_ERROR,1,x)
#define fatal(x...)     _log(LOGGER_FATAL,1,x)


//这一组宏 都不带锁
#define raw_nl(x...)    _rlog(LOGGER_RAW, 0,x);
#define debug_nl(x...)  _log(LOGGER_DEBUG,0,x);
#define info_nl(x...)   _log(LOGGER_INFO, 0,x);
#define warn_nl(x...)   _log(LOGGER_WARN, 0,x);
#define error_nl(x...)  _log(LOGGER_ERROR,0,x);
#define fatal_nl(x...)  _log(LOGGER_FATAL,0,x);


//这一组宏 不带时间显示,但是带锁
#define debug_nt(x...)  log_nt(LOGGER_DEBUG,1,x);
#define info_nt(x...)   log_nt(LOGGER_INFO, 1,x);
#define warn_nt(x...)   log_nt(LOGGER_WARN, 1,x);
#define error_nt(x...)  log_nt(LOGGER_ERROR,1,x);
#define fatal_nt(x...)  log_nt(LOGGER_FATAL,1,x);


#endif
