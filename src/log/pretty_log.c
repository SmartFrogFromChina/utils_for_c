#include "pretty_log.h"
#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

#define BLUE    "\033[34m"   //蓝色
#define GREEN   "\033[32m"   //绿色
#define PURPLE  "\033[35m"   //紫红色
#define YELLOW  "\033[33m"   //黄色
#define RED     "\033[31m"   //红色


char log_level_str[MAX_TYPE+1] = {0};
char log_file_path[MAX_SIZE]   = {0};


static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *name;
    char *color;
}params_t;

params_t params[] = {
{"FATAL",   RED},
{"ERROR",   YELLOW},
{"WARN",    PURPLE},
{"INFO",    GREEN},
{"DEBUG",   BLUE},
};


void get_current_time(char *ts,int size)
{
    char tmp[32];
    struct timeval tv={0,0};
    struct tm *lt = NULL;
    if(gettimeofday(&tv,NULL)<0) {return ;}
    lt = localtime(&tv.tv_sec);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", lt);
    //buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    snprintf(ts,size,"%s.%06ld",tmp,tv.tv_usec);   //显示微秒
}


/* 使用说明： ./test abc 123  -l xxxxxx -o /tmp/test.log
 * 命令行中有log的特征字符，即可启动log
 * LOG共分6级：debug、info、warn、error、fatal和raw
 * -l 从左到右分别与6级log对应，只要对应位不为0，则可打印该级别的log
 * -o 是将log信息写入文件中
*/
void pretty_log_init(int argc,char **argv)
{
    char ch = -1;
    while((ch = getopt(argc, argv, "l:o:")) != -1)
    {
        switch(ch)
        {
            case 'l': strcpy(log_level_str,optarg); break; 
            case 'o': strcpy(log_file_path,optarg); break;
        }
    }
}


void log_log(char level,char lock,char *cmd_str,char *path,char *time,const char *file,const char *function,const int line,char *fmt, ...) 
{
    if(0 != cmd_str[level])
    {
        if(lock)
            pthread_mutex_lock(&log_lock); 

        va_list args;
        fprintf(stderr,"%s %s%-5s\033[0m %s:%s:%d ",time+11,params[level].color,params[level].name,file,function,line);
        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);
        fprintf(stderr,"\n");
        

        if(path[0])
        {
            FILE *fp=NULL;
            if(!(fp=fopen(path,"a+")))
            {
                printf("fopen %s fail:%s\n",path,strerror(errno));
            }
            else
            {
                va_list args;
                fprintf(fp,"%s %-5s %s:%s:%d ",time,params[level].name,file,function,line);
                va_start(args,fmt);
                vfprintf(fp,fmt,args);
                va_end(args);
                fprintf(fp,"\n");
                if(fclose(fp)){
                    printf("error:close %s fail:%s\n",path,strerror(errno));
                }
            }
        }
        if(lock)
            pthread_mutex_unlock(&log_lock);       
    }
}



inline void raw_log(char level,char lock,char *cmd_str,const char *path,const char *fmt,...){
    if(0 != cmd_str[level])
    {
        if(lock)
            pthread_mutex_lock(&log_lock);
        
        va_list args;
        va_start(args,fmt);
        vfprintf(stderr,fmt,args);
        va_end(args);

        if(path[0])
        {
            FILE *fp=NULL;
            if(!(fp=fopen(path,"a+")))
            {
                printf("fopen %s fail:%s\n",path,strerror(errno));
            }
            else
            {
                va_list args;
                va_start(args,fmt);
                vfprintf(fp,fmt,args);
                va_end(args);
                fclose(fp);
            }
        }

        if(lock)
            pthread_mutex_unlock(&log_lock);       
    }
}


