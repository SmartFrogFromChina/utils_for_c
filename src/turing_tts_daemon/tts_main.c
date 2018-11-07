#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include "request_and_response.h"


/* 图灵TTS参数. */
typedef  struct 
{
    int tts;        
    /* file type
     * 0：pcm_8K_16bit（默认）
     * 1：mp3_64
     * 2：mp3_24
     * 3：mp3_16
     * 4：amr_nb
     * 5：pcm_24K_16bit
     * 6：pcm_16K_16bit
    */
    int tts_lan;
    /* language
     * 0:中文合成（默认）
     * 1:英文合成
    */
    int speed;
    /* tts语速设置，取值范围1~9，默认为5 */
    int pitch;
    /* ts语调设置，取值范围1~9，默认为5 */
    int volume;
    /* tts音量，取值范围1～9，默认 5 */
    int tone;
    /* tts发音人选择 取值范围0~15，20~22 默认 0 
     *  0	xixi，女声
     *  1	liangjiahe，女声
     *  2	cartoonjing，女声
     *  3	wangjing，女声
     *  4	xiaokun，女声
     *  5	haobo，男声
     *  6	diaoxiong，男声
     *  7	jiangman，女声
     *  8	shenxu，男声
     *  9	wangjing.v9，女声
     *  10	xiaokun.v9，女声
     *  11	xumengjuan，女声
     *  12	zhaqian，女声
     *  13	zhangnan，女声
     *  14	cameal，女声，英文合成使用
     *  15	barron，男声，英文合成使用
     *  20	智娃
     *  21	阿Q
     *  22	慧听
    */
}tts_t;

char *filename = NULL;
int file_type = 0;

char *get_host_from_url(char *url)
{
    if(url)
    {
        char *host_start = strstr(url,"http://");
        if(host_start)
        {
            host_start += strlen("http://");
            char *host_end = strstr(host_start,"/");
            if(host_end)
            {
                char buf[128] = {0};
                strncpy(buf,host_start,host_end-host_start);
                return strdup(buf);
            }
        }
    }
    return NULL;
}

char *get_path_from_url(char *url)
{
    if(url)
    {
        int len = strlen(url);
        char *host_start = strstr(url,"http://");
        if(host_start)
        {
            host_start += strlen("http://");
            char *path_start = strstr(host_start,"/");
            if(path_start)
            {
                char buf[128] = {0};
                strncpy(buf,path_start,url+len-path_start);
                return strdup(buf);
            }
        }
    }
    return NULL;
}


int download_file_by_url(char *url)
{
    if(!url)
    {
        return -1;
    }
    int ret = 0;
    char name[64] = {0};
    //printf("url=[%s]\n",url);
    char *suffix = strrchr(url,'.');
    //printf("suffix=[%s]\n",suffix);
    sprintf(name,"%s%s",filename,suffix);
    
    char *host = get_host_from_url(url);
    char *path = get_path_from_url(url);
    int fd = get_socket_fd(host);
    if(fd < 0)
        return -1;
    ret = build_turing_download_request(fd,path,host);
    save_file(fd,name);
    
    return 0;
}


char * ParseJson(char * str)
{
    char *code_split = "\"code\":";
    char *code_start = strstr(str,code_split);
    if(code_start)
    {
        code_start += strlen(code_split);
        char *code_end = strstr(code_start,",");
        if(code_end)
        {
            char buf[8] = {0};
            strncpy(buf,code_start,code_end-code_start);
            if(atoi(buf) != 200)
            {
                return NULL;
            }
        }
    }
    
    char *url_split = "\"http";
    char *url_start = strstr(str,url_split);
    if(url_start)
    {
        url_start += 1;
        char *url_end = strstr(url_start,"\"");
        char *url = calloc(256,1);
        if(url)
        {
            strncpy(url,url_start,url_end-url_start);
            return url;
        }
    }
    return NULL;
}

char *CreateTuringTtsJson(char *key ,char *uid, char *token,char *text, tts_t *param)
{
    char *parameter ="{"
                        "\"parameters\":"
                        "{"
                            "\"ak\": \"%s\","
                            "\"uid\": \"%s\","
                            "\"token\":\"%s\","
                            "\"text\": \"%s\","
                            "\"tts\": %d,"
                            "\"tts_lan\": %d,"
                            "\"speed\": %d,"
                            "\"pitch\": %d,"
                            "\"tone\":%d,"
                            "\"volume\": %d"
                        "}"
                    "}";
    
    char * json_str = calloc(1024,1);
    if(json_str)
    {
        snprintf(json_str,1024,parameter,key,uid,token,text,param->tts,\
            param->tts_lan,param->speed,param->pitch,param->tone,param->volume);
        return json_str;
    }
    return NULL;
}



char * get_turing_tts_url(int tone,char *text)
{
	int ret = 0;
	char *json = NULL;
	char *token = NULL;
	char *apiKey = NULL;
	char *uid = NULL;
	tts_t param;
    char *purl = NULL;
    int fd = 0;
	char *host="smartdevice.ai.tuling123.com";
	char *input_text = NULL;
    
	memset(&param , 0 , sizeof(tts_t));
	param.tts     = file_type;
	param.tts_lan = 0;
	param.speed   = 5;
	param.pitch   = 5;
	param.tone 	  = tone;
	param.volume  = 9;

    
	apiKey  = "8268ff3bce3e45c7b10a6d49a0fddcd5";
	uid     = "9C5DE14C6503BCF56821D7A41DA23B4D";
	token   = "2af62825b9cd49568cc55a6256a86239";
	
	json = CreateTuringTtsJson(apiKey ,uid, token, text, &param);
	if(json == NULL) 
    {
		ret = -1;
		goto exit;
	}
	//printf("json:%s\n", json);
    fd = get_socket_fd(host);
    if(fd < 0)
        return NULL;
	ret = build_turing_tts_request(fd, host, json);
	if(ret < 0)
		goto exit;
	getResponse(fd, &input_text);
    close(fd);
	purl = ParseJson(input_text);
    //printf("input_text=[%s]\n",input_text);
exit:
	if(json)
		free(json);
	
	if(input_text)
        free(input_text);
	return purl;
	
}


int main(int argc,char **argv)
{
    int i = 0;
    char *url = NULL;
    if(argc < 4)
    {
        printf("usage:%s tone filetype text1 [text2] [text3] [text4]\n",basename(argv[0]));
        printf("\033[30mtone:\033[0m\n");
        printf("*  0	xixi，         女声（默认）\n");
        printf("*  1	liangjiahe，   女声\n");
        printf("*  2	cartoonjing，  女声\n");
        printf("*  3	wangjing，     女声\n");
        printf("*  4	xiaokun，      女声\n");
        printf("*  5	haobo，        男声\n");
        printf("*  6	diaoxiong，    男声\n");
        printf("*  7	jiangman，     女声\n");
        printf("*  8	shenxu，       男声\n");
        printf("*  9	wangjing.v9，  女声\n");
        printf("*  10	xiaokun.v9，   女声\n");
        printf("*  11	xumengjuan，   女声\n");
        printf("*  12	zhaqian，      女声\n");
        printf("*  13	zhangnan，     女声\n");
        printf("*  14	cameal，       女声，英文合成使用\n");
        printf("*  15	barron，       男声，英文合成使用\n");
        printf("*  20	智娃\n");
        printf("*  21	阿Q\n");
        printf("*  22	慧听\n\n");
        printf("\033[31mfiletype:\033[0m\n");
        printf("* 0：pcm_8K_16bit（默认）\n");
        printf("* 1：mp3_64\n");
        printf("* 2：mp3_24\n");
        printf("* 3：mp3_16\n");
        printf("* 4：amr_nb\n");
        printf("* 5：pcm_24K_16bit\n");
        printf("* 6：pcm_16K_16bit\n\n");        
        
        return 0;
    }
    int tone = atoi(argv[1]);
    file_type = atoi(argv[2]);
    
    for(i=3; i<= argc-1; i++)
    {
        url = get_turing_tts_url(tone,argv[i]);
        filename = argv[i];
        download_file_by_url(url);
        free(url);
    }
    return 0;
}
