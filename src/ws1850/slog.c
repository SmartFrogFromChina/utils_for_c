#include "slog.h"

#define CCHIP_MIN(x,y) ((x)<(y)?(x):(y))

//#include "function/common/misc.h"
log_ctrl_t logCtrlArray[MAX_TYPE]={
	[_ERR]	={"ERR"		,Hred		},
	[_WAR]	={"WAR"		,Hyellow	},
	[_INF]	={"INF"		,Hgreen		},
	[_DBG]	={"DBG"		,Hblue		},
	[_TRC]	={"TRC"		,Hpurple	},
};
static FILE *fp=NULL;

char log_ctrl_set[MAX_TYPE+1]="111";

pthread_mutex_t line_lock = PTHREAD_MUTEX_INITIALIZER;

#if 0
void get_time_ms(char *ts,int size){
	struct timeval tv={0};
	if(gettimeofday(&tv,NULL)<0){
		return;
	}
	snprintf(ts,size,"%013lu",tv.tv_sec*1000+tv.tv_usec/1000);
}
#else
void get_current_time(char *ts,int size)
{
    char tmp[32];
    struct timeval tv={0,0};
    struct  timezone   tz={0,0};
    struct tm *lt = NULL;
    if(gettimeofday(&tv,NULL)<0) {return ;}
    lt = localtime(&tv.tv_sec);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", lt);
    //buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    snprintf(ts,size,"%s.%06ld",tmp,tv.tv_usec);   //显示微秒
}
#endif
int log_init(char *ctrl,char *path)
{
	if(ctrl){
		strncpy(log_ctrl_set,ctrl,CCHIP_MIN(sizeof(log_ctrl_set),strlen(ctrl)));
	}
	if(path){
		pthread_mutex_lock(&line_lock);
		if(!(fp=fopen(path,"a"))){
			printf("fopen %s fail,errno=%d\n",path,errno);
			fp=stdout;
		}
		pthread_mutex_unlock(&line_lock);
	}
}

inline void slog(log_type_t n,char lock,char *log_ctrl_set,const char *ts,\
					const char *file,const int line,const char *fmt,...){						
	if('1' == log_ctrl_set[n]){
		if(lock)pthread_mutex_lock(&line_lock);	
		if(!fp){
			fp=stdout;
		}
		va_list args;
		fprintf(fp,"[%s %s%s %s:%d]:\033[0m",ts,logCtrlArray[n].color,logCtrlArray[n].name,get_last_name(file),line);			
		va_start(args,fmt);
		vfprintf(fp,fmt,args);
		va_end(args);
		fprintf(fp,"\n");
        fflush(fp);
		if(lock)pthread_mutex_unlock(&line_lock);
	}
}
inline void raw_log(log_type_t n,char lock,char *log_ctrl_set,const char *fmt,...){
	if('1'==log_ctrl_set[n]){
		if(lock)pthread_mutex_lock(&line_lock);
		if(!fp){
			fp=stdout;
		}
		va_list args;
		va_start(args,fmt);
		vfprintf(fp,fmt,args);
		va_end(args);
		if(lock)pthread_mutex_unlock(&line_lock);		
	}
}