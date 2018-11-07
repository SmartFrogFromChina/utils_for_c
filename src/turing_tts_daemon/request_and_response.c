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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "request_and_response.h"
#include "wav.h"


static char upload_head[] = 
	"POST /speech/chat HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
	"Content-Length: %d\r\n"
    "Cache-Control: no-cache\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n"
	"Content-Type: multipart/form-data; boundary=%s\r\n"
    "Accept: */*\r\n"
    "Accept-Language: en-US,en;q=0.8,zh-CN;q=0.6,zh;q=0.4,zh-TW;q=0.2,es;q=0.2\r\n"
    "\r\n";

static char tts_head[] = 
	"POST /speech/v2/tts HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
	"Content-Length: %d\r\n"
    "Cache-Control: no-cache\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36\r\n"
	"Content-Type: application/json\r\n"
    "Accept: */*\r\n"
    "\r\n";

static char download_head[]=
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 Safari/537.36\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
    "Accept-Language: zh-CN,zh;q=0.9\r\n"
    "\r\n";



static char upload_parameters[] = 
	"Content-Disposition: form-data; name=\"parameters\"\r\n\r\n%s";

static char upload_speech[] = 
    "Content-Disposition: form-data; name=\"speech\"; filename=\"upload.wav\"\r\n"
    "Content-Type: application/octet-stream\r\n\r\n";


int get_socket_fd(char *host)
{
    int portno = 80;
    int sockfd, on=1;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    
	
    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { 
		printf("ERROR opening socket");
		return sockfd;
	}
	setsockopt (sockfd, SOL_TCP, TCP_NODELAY, &on, sizeof (on));

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

int safe_send(int sockfd, void *buf, size_t total_bytes)
{
    int ret = 0;
    int send_bytes, cur_len;
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
	int on = 1;
	
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
            goto recv_again;
        else
            printf("recv error, fd=%d, errno=%d %m", sockfd, errno);
    }
	setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &on, sizeof(int));  
    //printf("cur_len = %d",cur_len);
    return cur_len;
}


static void get_resp_header(const char *response, struct resp_header *resp)
{    
	/*获取响应头的信息*/
    
	char *pos = strstr(response, "HTTP/");    
	if (pos) {        
		//printf("status_code: %s", pos);
		sscanf(pos, "%*s %d", &resp->status_code);//返回状态码    
	}
	else 
	{
		resp->status_code = 0;
	} 
	
	pos = strstr(response, "Content-Type:");//返回内容类型    
	if (pos) {      
		//printf("Content-Type: %s", pos);
		sscanf(pos, "%*s %s", resp->content_type);    
	}
	
	
	pos = strstr(response, "Content-Length:");//内容的长度(字节)    
	if (pos) {       
		//printf("Content-Length: %s", pos);
		sscanf(pos, "%*s %ld", &resp->content_length); 
	}
	else 
		resp->content_length = 0;

    pos = strstr(pos,"\r\n\r\n"); //内容的开始
    if(pos){
        //printf("contents:%s",pos);
        pos += 4;
        resp->content_buf = pos;
    }
    else
        {printf("Can not find \r\n\r\n ");}
        
  
}


int getResponse(int socket_fd, char **text)
{
    char *response = NULL;
	char *code  = NULL;
	int length = 0,mem_size=1024;
	response = calloc(1, 1024);
	if(response == NULL)
		return -1;
    memset(response, 0, 1024);
    
    struct resp_header resp;
    int ret=0;

    int retryCount = 0;
    while (1)
    {
        ret = safe_recv(socket_fd, response+length, 1);
        if(ret < 0)
        {
            if(errno == EINTR ||errno == EAGAIN )
            {   
                continue;
            }
        }
        else if(ret == 0)
        {
            printf("connection closed by peer, fd=%d", socket_fd);
            break;
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
	while(1){		
		ret = safe_recv(socket_fd, code+length, resp.content_length-length);
		if(ret <= 0){
			break;
		}
		length += ret;
		
		if(length==resp.content_length)
			break;
	}
    *text = code;
exit:
	if(response)
		free(response);
	
	return length;
}
extern int file_type;

int save_file(int socket_fd,char *filename)
{
    char *response = NULL;
	char *code  = NULL;
	int length = 0,mem_size=1024;
	response = calloc(1, 1024);
	if(response == NULL)
		return -1;
    memset(response, 0, 1024);
    
    struct resp_header resp;
    int ret=0;

    int retryCount = 0;
    while (1)
    {
        ret = safe_recv(socket_fd, response+length, 1);
        if(ret < 0)
        {
            if(errno == EINTR ||errno == EAGAIN )
            {   
                continue;
            }
        }
        else if(ret == 0)
        {
            printf("connection closed by peer, fd=%d", socket_fd);
            break;
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

	get_resp_header(response,&resp);
    
	if(resp.status_code != 200 || resp.content_length == 0){
		length = -1;
		goto exit;
	}
	ret=0;
	length=0;
	code = (char *)calloc(1,resp.content_length);
	if(code == NULL){
		length = -1;
		goto exit;
	}
	while(1){		
		ret = safe_recv(socket_fd, code+length, resp.content_length-length);
		if(ret <= 0){
			break;
		}
		length += ret;
		//printf("length = %d\n",length);
		if(length==resp.content_length)
			break;
	}
    if(length == resp.content_length)
    {
        
        FILE *fp = NULL;
        int  len = 0;
        audio_para_t audio ={0};

        char *suffix = strstr(filename,".pcm");
        if(suffix)
        {
            *(suffix+1) = 'w';
            *(suffix+2) = 'a';
            *(suffix+3) = 'v';
        }
        fp = fopen(filename,"w+");
        if(!fp)
        {
            printf("open %s error\n",filename);
            goto exit;
        }
        if(strstr(filename,".wav"))
        {
            audio.channel = 1;
            audio.sample_accuracy = 16;
            audio.data_len = length;
            if(file_type == 0)
            {
                audio.sample_rate = 8000;
            }
            else if(file_type == 5)
            {
                audio.sample_rate = 24000;
            }
            else if(file_type == 6)
            {
                audio.sample_rate = 16000;
            }
            len = set_wav_header(fp,&audio);
        }
        
        fseek(fp,len,SEEK_SET);
        fwrite(code,1,length,fp);
        fclose(fp);
        printf("\033[32m[%s]\033[0m 保存成功\n",filename);
    }

    free(code);    
    
exit:
	if(response)
		free(response);

	return length;
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
    safe_send(socketFd, header_data, ret);
	safe_send(socketFd, firstBoundary, strlen(firstBoundary));
	safe_send(socketFd, parameter_data, strlen(parameter_data));
    safe_send(socketFd, upload_speech, strlen(upload_speech));  
    

	int w_size=0,pos=0,all_Size=0;
	while(1) {
		
        pos = safe_send(socketFd,data+w_size,len-w_size);
		if(pos < 0) {
			ret = -1;
			break;
		}
        printf("pos = %d",pos);
		w_size += pos;
		all_Size += len;
		if( w_size== len ){
			w_size=0;
			break;
		}
	}
    ret= safe_send(socketFd, endBoundary, strlen(endBoundary));
exit:
	if(header_data)
		free(header_data);
	if(boundary)
		free(boundary);
	if(parameter_data)
		free(parameter_data);
	return ret;
}


int build_turing_tts_request(int socketFd,char *host, char *jsonStr)
{
    int ret = 0;
    char * header_data = calloc(4096,1);
    int content_length = strlen(jsonStr);
    
    ret = snprintf(header_data,4096, tts_head, host,content_length);
    ret = safe_send(socketFd, header_data, ret);         //发送头部信息
    ret = safe_send(socketFd, jsonStr, content_length);  //发送body数据,数据不足16K，一次可以发送完
    free(header_data);

    return ret;
}

int build_turing_download_request(int socketFd,char *path,char *host)
{
    int ret = 0;
    char * header_data = calloc(4096,1);
    
    ret = snprintf(header_data,4096, download_head, path,host);
    ret = safe_send(socketFd, header_data, ret);         //发送头部信息
    free(header_data);

    return ret;
}


