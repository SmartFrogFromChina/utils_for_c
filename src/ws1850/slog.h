#ifndef _SLOG_H_
#define _SLOG_H_ 1
#include "com_log.h"
typedef enum log_type_t{
	MIN_TYPE=-1,
	_RAW,	
	_ERR,
	_WAR,
	_INF,	
	_DBG,
	_TRC,
	MAX_TYPE
}log_type_t;

typedef struct log_ctrl_t{
	char *name;
	char *color;
}log_ctrl_t;

void get_current_time(char *ts,int size);
void get_time_ms(char *ts,int size);
int log_init(char *ctrl,char *path);

#define MAX_PNL 	64
extern char log_ctrl_set[MAX_TYPE+1];

void slog(
		log_type_t type,		//log 类型（级别）：
		char lock_en,			//是否启用互斥锁
		char *log_ctrl_set,		//log 开关控制
		const char *ts,			//时间字符串
		const char *file,		//调用该函数的文件名
		const int line,			//调用该函数的行号
		const char *fmt,		//用于输出的格式化字符串
		...);
					
void raw_log(
		log_type_t type,
		char lock,
		char *log_ctrl_set,
		const char *fmt,
		...);
				
#define tlog(type,lock,x...) ({\
	char ts[32]="";\
	get_current_time(ts,sizeof(ts));\
	slog(type,lock,log_ctrl_set,ts,__FILE__,__LINE__,x);\
})

#define dlog(type,lock,x...) slog(type,lock,log_ctrl_set,"",__FILE__,__LINE__,x);

#define rlog(type,lock,x...) raw_log(type,lock,log_ctrl_set,x);

//这一组宏 都是带锁的，适用于多线程程序调试，单不能用于信号回调函数中
#define raw(x...) 	rlog(_RAW,1,x);//以原始方式打印，但是带锁，也可以进行开关操作
#define err(x...) 	tlog(_ERR,1,x);
#define war(x...) 	tlog(_WAR,1,x);
#define inf(x...) 	tlog(_INF,1,x);
#define dbg(x...) 	tlog(_DBG,1,x);
#define trc(x...) 	tlog(_TRC,1,x);

//这一组宏 都是不带锁的，适用于信号回调函数中，单不能用于多线程程序调试
#define raw_nl(x...) 	rlog(_RAW,0,x);
#define err_nl(x...) 	dlog(_ERR,0,x);
#define war_nl(x...) 	dlog(_WAR,0,x);
#define inf_nl(x...) 	dlog(_INF,0,x);
#define dbg_nl(x...) 	dlog(_DBG,0,x);
#define trc_nl(x...) 	dlog(_TRC,0,x);


#define err_nt(x...) 	slog(_ERR,1,x);
#define war_nt(x...) 	slog(_WAR,1,x);
#define inf_nt(x...) 	slog(_INF,1,x);
#define dbg_nt(x...) 	slog(_DBG,1,x);
#define trc_nt(x...) 	slog(_TRC,1,x);

#define show_errno(ret, msg) ({\
	int num=(ret);\
	if(!num)num=errno;\
	err("%s failure,errno:%d[%s]",msg,num,strerror(num));\
})

#define fatal(x...)     tlog(_ERR,1,x);
#define error(x...)     tlog(_ERR,1,x);
#define warning(x...)   tlog(_WAR,1,x);
#define info(x...)      tlog(_INF,1,x);
#define debug(x...)     tlog(_DBG,1,x);
#define trace(x...)     tlog(_TRC,1,x);

#endif