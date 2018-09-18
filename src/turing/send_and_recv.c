#include <sys/unistd.h>
#include <sys/stat.h>
#include <stdio.h> /* printf, sprintf */
#include <dirent.h>
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>


static char upload_head[] = 
	"POST /speech/chat HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
	"Content-Length: %d\r\n"
    "Cache-Control: no-cache\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n"
	"Content-Type: multipart/form-data; boundary=%s\r\n"
    "Accept: */*\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: en-US,en;q=0.8,zh-CN;q=0.6,zh;q=0.4,zh-TW;q=0.2,es;q=0.2\r\n"
    "\r\n";


static char upload_parameters[] = 
	"Content-Disposition: form-data; name=\"parameters\"\r\n\r\n%s";

static char upload_speech[] = 
    "Content-Disposition: form-data; name=\"speech\"; filename=\"upload.wav\"\r\n"
    "Content-Type: application/octet-stream\r\n\r\n";



char * json_str = "{\
    \"ak\": \"94616bceca4a4d94a18c0579cea97fb7\",\
    \"uid\": \"4997aee0609bfc5e66f6757796b35be0\", \"asr\": 0, \"tts\": 1,\
    \"speed\": 5, \"realTime\": 1, \"index\": %d,\
    \"identify\": \"%s\", \"flag\": 3,\
    \"token\": \"%s\", \"type\": 0, \"tone\": 0,\
    \"volume\": 9 }";



char g_identify[48] = {0};
int g_send_fd = 0;
static char *turing_host = "smartdevice.ai.tuling123.com";
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
char g_token[36] = {0};




#define USER_SAFE_SOCKET
//#undef USER_SAFE_SOCKET
struct resp_header{    
    int status_code;        //HTTP/1.1 '200' OK    
    char content_type[128]; //Content-Type: application/gzip    
    long content_length;    //Content-Length: 11683079
    char *content_buf;
};



void turing_log(const char *function,int line, const char *fmt, ...) 
{
    pthread_mutex_lock(&lock);
    struct tm *lt = NULL;
    va_list args;
    char buf[32];
    char tmp[16];
    struct timeval tv={0,0};
    gettimeofday(&tv,NULL);
    lt = localtime(&tv.tv_sec);
    strftime(tmp, sizeof(tmp), "%H:%M:%S", lt);
    sprintf(buf,"%s.%06ld",tmp,tv.tv_usec);   //显示微秒

    fprintf(stderr, "%s %s:%d ",buf,function,line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    pthread_mutex_unlock(&lock);
}


#define printf(...)      do { turing_log(__func__, __LINE__, __VA_ARGS__); }while(0)

void get_token()
{
    FILE *fp = fopen("token.txt","r");
    if(!fp)
    {
        strcpy(g_token,"0");
    }
    else
    {
        fgets(g_token,36,fp);
        //printf("g_token = [%s]",g_token);
        fclose(fp);
    }
    
}

void save_token(char *str)
{
    FILE *fp = fopen("token.txt","w+");
    if(!fp)
    {
        printf("error to create token.txt");
        return ;
    }
    else
    {
        char *split= "token\":\"";
        char *start = strstr(str,split);
        if(start)
        {
            start += strlen(split);
            char *end = strstr(start,"\",\"");
            if(end)
            {
                strncpy(g_token,start,end-start);
                fputs(g_token,fp);
                //printf("token = [%s]",g_token);
            }
        }
        fclose(fp);
    }
}


int get_socket_fd(char *host)
{
    int portno = 80;
    int sockfd;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    
    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { 
		printf("ERROR opening socket");
		return sockfd;
	}

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) {
		printf("ERROR, no such host");
		close(sockfd);
		return -1;
    }
    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting");
		close(sockfd);
		return -1;
	}
	
    return sockfd;
    
}

//send(socketFd, header_data, ret,0);

int safe_send(int sockfd, void *buf, size_t total_bytes)
{
    int ret = 0;
    int send_bytes, cur_len;
    //trace("prepare to send %d bytes",total_bytes);
    for (send_bytes = 0; send_bytes < total_bytes; send_bytes += cur_len)
    {
        cur_len = send(sockfd, buf + send_bytes, total_bytes - send_bytes, 0);
        if (cur_len == 0)
        {
            printf("send tcp packet error, fd=%d, %m", sockfd);
            return -1;
        }
        else if (cur_len < 0)
        {
            if (errno == EINTR)
                cur_len = 0;
            else if (errno == EAGAIN)
            {
                printf("errno == EAGAIN,send_bytes=%d",send_bytes);
                return send_bytes;
            }
            else
            {
                printf("send tcp packet error, fd=%d, %m", sockfd);
                return -1;
            }
        }
    }
    //printf ("send: fd=%d, len=%d", sockfd, send_bytes);
    return send_bytes;
}

int safe_recv(int sockfd, void *buf, size_t nbytes)
{   
    int cur_len;
    //printf("prepare to receive %d bytes",nbytes);
recv_again:
    cur_len = recv (sockfd, buf, nbytes, 0);
    if (cur_len == 0)
    {
        printf("connection closed by peer, fd=%d", sockfd);
        return 0;
    }
    else if (cur_len < 0)
    {
        if (errno == EINTR || errno == EAGAIN)
        {
            printf("recv need continue");
            goto recv_again;
        }
            
        else
            printf("recv error, fd=%d, errno=%d %m", sockfd, errno);
    }
	//setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &on, sizeof(int));  
    //printf("cur_len = %d",cur_len);
    return cur_len;
}


static void get_resp_header(const char *response, struct resp_header *resp)
{    
	/*获取响应头的信息*/
    
	char *pos = strstr(response, "HTTP/");    
	if (pos) {        
		//debug("status_code: %s", pos);
		sscanf(pos, "%*s %d", &resp->status_code);//返回状态码    
	}
	else 
	{
		resp->status_code = 0;
	} 
	
	pos = strstr(response, "Content-Type:");//返回内容类型    
	if (pos) {      
		//debug("Content-Type: %s", pos);
		sscanf(pos, "%*s %s", resp->content_type);    
	}
	
	
	pos = strstr(response, "Content-Length:");//内容的长度(字节)    
	if (pos) {       
		//debug("Content-Length: %s", pos);
		sscanf(pos, "%*s %ld", &resp->content_length); 
	}
	else 
		resp->content_length = 0;

    pos = strstr(pos,"\r\n\r\n")+4; //内容的开始
    if(pos){
        resp->content_buf = pos;
    }
    else
        {printf("Can not find rnrn ");}
        
  
}

int get_response_code(char *str)
{
    if(!str)
        return -1;
    if(strstr(str,"40000"))
        return -1;

    save_token(str);
    return 0;
}

int getResponse(int socket_fd, char **text)
{
    //char response[4096];
    char *response = NULL;
	char *code  = NULL;
	int length = 0,mem_size=1024;
	response = calloc(1, 1024);
	if(response == NULL)
		return -1;
    memset(response, 0, 1024);
    
    struct resp_header resp;
    int ret=0;
    //debug("");
    //人为的获取http头部信息，暂时不要数据部分。因为不知道数据长度
    //printf("receiving header start");
    while (1)
    {
#ifdef USER_SAFE_SOCKET
		ret = safe_recv(socket_fd, response+length, 1);
        //printf("received first byte");
#else
		ret = recv(socket_fd, response+length, 1,0);
#endif        
		if(ret < 0)
        {
            if(errno == EINTR ||errno == EAGAIN )
            {   
                printf("recv need continue");
                continue;
            }
        }
        else if(ret == 0)
        {
            printf("connection closed by peer, fd=%d", socket_fd);
            length = -1;
            goto exit;

        }
		
		int flag = 0;	
		int i;			
		for (i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
		if (flag == 4)			  
			break;		  
		length += ret;	
		if(length >= mem_size-1){
			break;
		}
	}
    //printf("receiving header over");
    //printf("response = [%s]",response);
	get_resp_header(response,&resp);
    
	if(resp.status_code != 200 || resp.content_length == 0){
		length = -1;
		goto exit;
	}

	code = (char *)calloc(1,resp.content_length+1);
	if(code == NULL){
		length = -1;
		goto exit;
	}
	ret=0;
	length=0;
    //printf("receiving contents start");
	while(1){		
#ifdef USER_SAFE_SOCKET
		ret = safe_recv(socket_fd, code+length, resp.content_length-length);
#else
		ret = recv(socket_fd, code+length, resp.content_length-length,0);
#endif		
		if(ret < 0){
            printf("ret < 0");
			break;
		}
        else if(ret == 0)
        {
            printf("connection closed by peer, fd=%d", socket_fd);
            length = -1;
            goto exit;
        }
		length += ret;
		
		if(length==resp.content_length)
			break;
	}
    //printf("receiving contents over.............");
    printf("contents is [%s]",code);
    ret = get_response_code(code);
    *text = code;
exit:
	if(response)
		free(response);
	
	return ret;
}
static void GetRandStr(char s[])
{
    struct timeval tv = {0,0};
    gettimeofday(&tv,NULL);
    sprintf(s,"%10ld%08ld",tv.tv_sec,tv.tv_usec);
}


int turingBuildRequest(int socketFd,char *host, char *data, int len, char *jsonStr)
{    
	int ret = 0;
	char *boundary_header = "------AiWiFiBoundary";
	char* end = "\r\n"; 			//结尾换行
	char* twoHyphens = "--";		//两个连字符
	char s[20] = {0};
	GetRandStr(s);
	char *boundary = malloc(strlen(boundary_header)+strlen(s) +1);
	memset(boundary, 0, strlen(boundary_header)+strlen(s) +1);
	strcat(boundary, boundary_header);
	strcat(boundary, s);

	char firstBoundary[64]={0};
	char secondBoundary[64]={0};
	char endBoundary[128]={0};

	sprintf(firstBoundary, "%s%s%s", twoHyphens, boundary, end);
	sprintf(secondBoundary, "%s%s%s%s", end, twoHyphens, boundary, end);
	sprintf(endBoundary, "%s%s%s%s%s", end, twoHyphens, boundary, twoHyphens, end);


	char *parameter_data = malloc(strlen(jsonStr)+ strlen(upload_parameters) + strlen(boundary) + strlen(end)*2 + strlen(twoHyphens) +1);
	memset(parameter_data, 0,strlen(jsonStr)+ strlen(upload_parameters) + strlen(boundary) + strlen(end)*2 + strlen(twoHyphens) +1);
	sprintf(parameter_data, upload_parameters, jsonStr);
	strcat(parameter_data, secondBoundary);

	int content_length = len+ strlen(boundary)*2 + strlen(parameter_data) + strlen(upload_speech) + strlen(end)*3 + strlen(twoHyphens)*3;
	char *header_data= malloc(4096);
	memset(header_data, 0, 4096);

	ret = snprintf(header_data,4096, upload_head, host, content_length, boundary);


	//header_data,boundary,parameter_data,boundary,upload_speech,fileData,end,boundary,boundary_end
#ifdef USER_SAFE_SOCKET
    safe_send(socketFd, header_data, ret);
	safe_send(socketFd, firstBoundary, strlen(firstBoundary));
	safe_send(socketFd, parameter_data, strlen(parameter_data));
    safe_send(socketFd, upload_speech, strlen(upload_speech));  
#else
    send(socketFd, header_data, ret,0);
	send(socketFd, firstBoundary, strlen(firstBoundary),0);
	send(socketFd, parameter_data, strlen(parameter_data),0);
    send(socketFd, upload_speech, strlen(upload_speech),0);    
#endif
    
	


	int w_size=0,pos=0,all_Size=0;
	while(1) {
#ifdef USER_SAFE_SOCKET
        pos = safe_send(socketFd,data+w_size,len-w_size);
#else
		pos = send(socketFd,data+w_size,len-w_size,0);
#endif        
		if(pos < 0) {
			ret = -1;
			break;
		}
		w_size += pos;
		all_Size += len;
		if( w_size== len ){
			w_size=0;
			break;
		}
	}
#ifdef USER_SAFE_SOCKET   
    ret= safe_send(socketFd, endBoundary, strlen(endBoundary));
#else
	ret= send(socketFd, endBoundary, strlen(endBoundary),0);
#endif
exit:
	if(header_data)
		free(header_data);
	if(boundary)
		free(boundary);
	if(parameter_data)
		free(parameter_data);
	return ret;
}


void getuuid(char *pDate)  
{  
    int flag, i;
	static unsigned int n = 0;
    srand((unsigned) time(NULL )+n);  
    for (i = 0; i < 36; i++)  
    {  
        flag = rand() % 2;  
        switch (flag)  
        {  
            case 0:  
                pDate[i] = 'a' + rand() % 6;  
                break;  
            case 1:  
                pDate[i] = '0' + rand() % 10;  
                break;  
            default:  
                pDate[i] = 'x';  
                break;  
        }  
	if(7 == i || 12 == i || 17 == i || 22 == i)
		pDate[++i] = '-';
    }  
    	pDate[36] = '\0';  
	n++;
}  





int send_data_to_turing(int index, char *pData, int iLength) 
{
    int ret = -1;
    char *text = NULL;
    char jsonData[1024] = {0};
    //printf("g_token = %s",g_token);
    sprintf(jsonData,json_str,index,g_identify,g_token);
    //printf("index = %d",index);
    
    if(NULL == jsonData) 
        return -1;
    //printf("upload start.............");
    ret = turingBuildRequest(g_send_fd, turing_host, pData, iLength,jsonData);
    //printf("upload finish .............");
    if(ret < 0) {
        goto EXIT;
    }
#if 0    
    //printf("recv start.............");
    ret = getResponse(g_send_fd, &text);
    //printf("recv finish ............");
    
    if (text && index < 0) {
        save_token(text);
        free(text);
    } 
#endif
EXIT:

	return ret;
}


int getTime()    
{
   struct timeval tv;    
   
   gettimeofday(&tv,NULL);    
   return (int)((tv.tv_sec %1000) * 1000 + tv.tv_usec / 1000);  
}



void * turing_recv_thread(void *arg)
{
    char *text = NULL;
    int ret = 1;
    while(ret){
        ret = getResponse(g_send_fd,&text);
        free(text);
        text = NULL;
    }
}


int main(int argc,char **argv)
{
    FILE *fp = NULL;
    int size = 0;
    int start = 0;
    int finish = 0;
    int start_bak = 0;
    int ret = 0;
    if(argc != 3)
    {
        printf("Usage:[%s filename upload_size_once]  EX:%s 1.wav 16 means upload 16*1024 bytes every time",argv[0],argv[0]);
        return 0;
    }
    else
    {
        size = atoi(argv[2]) * 1024;
        fp = fopen(argv[1],"r");
        if(!fp)
        {
            printf("Can not open file");
            return -1;
        }
    }
    printf("开始测试\n");
    int index = 0;
    char *text = NULL;
    getuuid(g_identify);
    get_token();
    g_send_fd = get_socket_fd(turing_host);
    //printf("g_send_fd = %d",g_send_fd);
    pthread_t pid = 0;
    
    pthread_create(&pid,NULL,turing_recv_thread,NULL);
    
    char *data_buf = calloc(size,1);
    if(!data_buf)
    {
        goto CALLOC_ERROR;
    }
    start = getTime();
    //printf("start = %d",start);
    printf("start = %d.%d",start/1000,start%1000);
    int nread = fread(data_buf,1,size,fp);
    
    while(nread > 0)
    {   
        index++;
        if(nread != size)
        {
            index = -index;
        }
        //start = getTime();
        send_data_to_turing(index,data_buf,nread);
        //finish = getTime();
        //printf("\033[31m第[%d]次耗时：%ld.%ld\033[0m",index,(finish-start)/1000,(finish-start)%1000);
        nread = fread(data_buf,1,size,fp);
    }

    pthread_join(pid,NULL);
    finish = getTime();
    printf("finish = %d.%d",finish/1000,finish%1000);
    printf("测试结束，总耗时：%d.%d",(finish-start)/1000,(finish-start)%1000);
    
CALLOC_ERROR:
    fclose(fp);
    free(data_buf);  
    data_buf = NULL;
    close(g_send_fd);

    return 0;
}

