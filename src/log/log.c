#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "log.h"

static struct {
    FILE *fp;
    int level;
    int quiet;
    pthread_mutex_t lock;    
}Log;


static const char *level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
    "\033[34m",   //蓝色
    "\033[36m",   //青蓝色
    "\033[32m",   //绿色
    "\033[33m",   //黄色
    "\033[31m",   //红色
    "\033[35m"    //紫红色
};
#endif


int log_init(int level,int quiet)
{
    if(pthread_mutex_init(&Log.lock, NULL) != 0) 
    {
        perror("Mutex initialization failed!\n");
        return -1;
    }
    Log.level = level;
    Log.quiet = quiet ? 1 : 0;
    
    return 0;
}

void log_deinit()
{
    if(pthread_mutex_destroy(&Log.lock) != 0) 
    {
        perror("Mutex destory failed!\n");
        return ;
    }
}

int file_log_init(char *file_name) 
{
    Log.fp = fopen(file_name, "a+"); 
    if (NULL == Log.fp) 
    {
        error("open %s failed", file_name);
        return -1;
    }
	
	return 0;
}
void file_log_deinit()
{
		if(Log.fp)
			fclose(Log.fp);
}

void log_log(int level, const char *file,const char *function,int line, const char *fmt, ...) 
{
  if (level < Log.level) 
  {
    return;
  }

  pthread_mutex_lock(&Log.lock);

  struct tm *lt = NULL;

  if (!Log.quiet) 
  {
    va_list args;
    char buf[32];
    
#ifdef LOG_USE_SECOND    
    time_t t = time(NULL);
    lt = localtime(&t);
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#else
    char tmp[16];
    struct timeval tv={0,0};
    gettimeofday(&tv,NULL);
    lt = localtime(&tv.tv_sec);
    strftime(tmp, sizeof(tmp), "%H:%M:%S", lt);
    sprintf(buf,"%s.%ld",tmp,tv.tv_usec);   //显示微秒
    
#endif    
    
#ifdef LOG_USE_COLOR
    fprintf(stderr, "%s %s%-5s\033[0m \033[90m%s:%s:%d:\033[0m ",buf, level_colors[level], level_names[level], file,function,line);
#else
    fprintf(stderr, "%s %-5s %s:%s:%d: ", buf, level_names[level], file, function, line);
#endif
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
  }

  if (Log.fp) 
  {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(Log.fp, "%s %-5s %s:%s:%d: ", buf, level_names[level], file,function, line);
    va_start(args, fmt);
    vfprintf(Log.fp, fmt, args);
    va_end(args);
    fprintf(Log.fp, "\n");
    fflush(Log.fp);    //必须加上，否则无法及时将log写入文件
  }
  
  pthread_mutex_unlock(&Log.lock);
}