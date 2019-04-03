#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <dirent.h>

#include "request_and_response.h"

#define TTS_VERSION     "1.0.0"
#define AUTHOR_EMAIL    "wheretogo0815@163.com"

/* 图灵TTS参数. */
typedef  struct 
{
    char *apikey;
    //apiKey,用于权限验证
    
    char *userid;
    
    //设备ID加密后的字符串，只支持数字和字母（0-9,a-z,A-Z)(参考AIWIFI接入)
    char *token;
    //请求令牌，首次请求可以为空。(参考AIWIFI接入)

    char *text;
    //需要合成的文本
    
    char * tts;        
    /* file type
     * 0：pcm_8K_16bit（默认）
     * 1：mp3_64
     * 2：mp3_24
     * 3：mp3_16
     * 4：amr_nb
     * 5：pcm_24K_16bit
     * 6：pcm_16K_16bit
    */
    char * tts_lan;
    /* language
     * 0:中文合成（默认）
     * 1:英文合成
    */
    char * speed;
    /* tts语速设置，取值范围1~9，默认为5 */
    char * pitch;
    /* ts语调设置，取值范围1~9，默认为5 */
    char * volume;
    /* tts音量，取值范围1～9，默认 5 */
    char * tone;
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
    char *output;
    //合成文件的保存目录
    char *path;
    //文件路径
    char *filename;
    //文件名称
    char *tts_url; 
    //合成后的tts路径
    int verbose;
}tts_t;


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


int download_file_by_url(tts_t *options)
{
    if(!options->tts_url)
    {
        return -1;
    }
    int ret = 0;
    char name[4096] = {0};
    //printf("url=[%s]\n",url);
    char *suffix = strrchr(options->tts_url,'.');
    //printf("suffix=[%s]\n",suffix);
    if(options->filename)
        sprintf(name,"%s%s",options->filename,suffix);
    else
        sprintf(name,"%s%s",options->text,suffix);
    //printf("name = %s\n",name);
    char *host = get_host_from_url(options->tts_url);
    char *path = get_path_from_url(options->tts_url);
    int fd = get_socket_fd(host);
    if(fd < 0)
        return -1;
    ret = build_turing_download_request(fd,path,host);
    save_file(fd,name,options->tts);
    free(options->tts_url);
    
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
                switch(atoi(buf))
                {
                    case 40001:printf("字段错误");break;      
                    case 40002:printf("非法字段");break;      
                    case 40003:printf("字段为空或错误");break;
                    case 40005:printf("文本转语音失败");break;
                    case 40007:printf("无效token");break;     
                    case 40008:printf("apikey过期");break;    
                    case 40012:printf("拒绝请求");break;      
                    case 40013:printf("请求超出限制");break;  
                    case 49999:printf("未知错误");break;      
                    case 42000:printf("合成tts的文本为空");break;
                }
                printf(",合成出错\n");
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

char *CreateTuringTtsJson(tts_t *param)
{
    char *parameter ="{"
                        "\"parameters\":"
                        "{"
                            "\"ak\": \"%s\","
                            "\"uid\": \"%s\","
                            "\"token\":\"%s\","
                            "\"text\": \"%s\","
                            "\"tts\": %s,"
                            "\"tts_lan\": %s,"
                            "\"speed\": %s,"
                            "\"pitch\": %s,"
                            "\"tone\":%s,"
                            "\"volume\": %s"
                        "}"
                    "}";
    
    char * json_str = calloc(4096,1);
    if(json_str)
    {
        snprintf(json_str,4096,parameter,param->apikey,param->userid,param->token,param->text,param->tts,\
            param->tts_lan,param->speed,param->pitch,param->tone,param->volume);
        return json_str;
    }
    return NULL;
}

void print_usage()
{
    printf( "用法:\n"
            "tts [选项...] \n\n"
            "例子：\n"
            "tts -f 5 -t 22 -e 今天日子不错\n"
            "tts -c speech.txt\n"
            "......\n"
            "选项:\n");
    printf( "   -h  显示帮助信息\n"
            "   -c  加载文本合成的文件。文件格式[生成的文件名,要合成的文本].Ex:[connected,联网成功]\n"
            "   -e  需要合成的文本\n"
            "   -f  需要合成的音频的格式(默认 0) \n"
            "       * 0：pcm_8K_16bit\n"
            "       * 1：mp3_64\n"
            "       * 2：mp3_24\n"
            "       * 3：mp3_16\n"
            "       * 4：amr_nb\n"
            "       * 5：pcm_24K_16bit\n"
            "       * 6：pcm_16K_16bit\n"
            "   -k  apiKey,用于权限验证\n"
            "   -l  需要合成的语种（目前支持中文和英文）(默认 0)\n"
            "       * 0:中文合成\n"
            "       * 1:英文合成\n"
            "   -o  tts文件保存目录（默认 tts_files）\n"
            "   -p  tts语调设置，取值范围1~9，默认为5\n"
            "   -s  tts语速设置，取值范围1~9，默认为5\n"
            "   -t  tts发音人选择 取值范围20~22（推荐)，0~15，默认0\n"
            "       *  0   xixi，女声(默认)\n"
            "       *  1   liangjiahe，女声\n"
            "       *  2   cartoonjing，女声\n"
            "       *  3   wangjing，女声\n"
            "       *  4   xiaokun，女声\n"
            "       *  5   haobo，男声\n"
            "       *  6   diaoxiong，男声\n"
            "       *  7   jiangman，女声\n"
            "       *  8   shenxu，男声\n"
            "       *  9   wangjing.v9，女声\n"
            "       *  10  xiaokun.v9，女声\n"
            "       *  11  xumengjuan，女声\n"
            "       *  12  zhaqian，女声\n"
            "       *  13  zhangnan，女声\n"
            "       *  14  cameal，女声，英文合成使用\n"
            "       *  15  barron，男声，英文合成使用\n"
            "       *  20  智娃\n"
            "       *  21  阿Q\n"
            "       *  22  慧听\n"
            "   -u  设备ID加密后的字符串\n"
            "   -v  tts音量，取值范围1～9，默认 5\n"
            "   -V  显示调试信息\n"
            "文本合成语音程序（V%s）由图灵提供技术支持 http://docs.turingos.cn/ai-wifi/tts\n"
            "bug提交：%s\n\n",TTS_VERSION,AUTHOR_EMAIL);


}


int get_turing_tts_url(tts_t *option)
{
	int ret = 0;
	char *json = NULL;
    int fd = 0;
	char *host="smartdevice.ai.tuling123.com";
	char *input_text = NULL;

	if(!option->text)
    {
        printf("参数错误：请按照格式输入要合成的文本\n\n");
        print_usage();
        return -1;
    }   
    json = CreateTuringTtsJson(option);
	if(json == NULL) 
    {
		ret = -1;
		goto exit;
	}
    if(option->verbose)
	    printf("json:%s\n", json);
    fd = get_socket_fd(host);
    if(fd < 0)
        return -1;
	ret = build_turing_tts_request(fd, host, json);
	if(ret < 0)
		goto exit;
	getResponse(fd, &input_text);
    close(fd);
    if(option->verbose)
        printf("recv=[%s]\n",input_text);
	option->tts_url = ParseJson(input_text);
exit:
	if(json)
		free(json);
	
	if(input_text)
        free(input_text);
	return 0;
	
}

int params_init(tts_t * tts)
{
    tts->apikey = "ece75b072cb74187b14c59e96aac51b6";
    tts->userid = "61f47f6e1d232860b005fff2a7b5fc99";
    tts->token  = "2af62825b9cd49568cc55a6256a86239";
    tts->text   = NULL;
    tts->tts    = "0";
    tts->tts_lan= "0";
    tts->speed  = "5";
    tts->pitch  = "5";
    tts->tone   = "0";
    tts->volume = "5";
    tts->output ="tts_files";
    tts->path   = NULL;
    tts->tts_url= NULL;
    tts->verbose= 0;
    
}

void parse_cmdline(int argc, char *argv[],tts_t *options)
{
    char ch = -1;
    while((ch = getopt(argc, argv, "c:e:f:h::k:l:o:p:s:t:u:v:T:V::")) != -1)
    {
        switch(ch)
        {
            case 'c':options->path    = optarg;break;
            case 'e':options->text    = optarg;break; 
            case 'f':options->tts     = optarg;break;
            case 'h':print_usage();          exit(0);
            case 'k':options->apikey  = optarg;break; 
            case 'l':options->tts_lan = optarg;break;
            case 'o':options->output  = optarg;break;
            case 'p':options->pitch   = optarg;break;
            case 's':options->speed   = optarg;break; 
            case 't':options->tone    = optarg;break;
            case 'u':options->userid  = optarg;break; 
            case 'v':options->volume  = optarg;break;
            case 'T':options->token   = optarg;break;
            case 'V':options->verbose = 1;     break;
        }
    }
}

void prepare_work_dir(tts_t *options)
{
    if(!options->output)
    {
        options->output="tts_files";
    }
    
    if(opendir(options->output) == 0)
    {
        printf("创建目录%s\n",options->output);
        int status = mkdir("tts_files", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(status)
        {
            printf("无法创建目录,请检查您的权限！\n");
            exit(0);
        }
    }
    
    chdir(options->output);
}
int parse_text_from_file(tts_t *options)
{
    #define SIZE (4 * 1024)
    FILE *fp = NULL;
    char buf[SIZE] = {0};
    char *name_start = NULL;
    char *name_end   = NULL;
    char *name_split = ",";
    char *text_start = NULL;
    char *text_end = NULL;
    char *text_split   = "]";
    char filename[SIZE]= {0};     
    char text[SIZE]= {0};
    
    if(!options->path)
    {
        return 1;
    }
        

    if(NULL == (fp = fopen(options->path,"r")))
    {
        printf("文件%s打开失败，请检查文件是否存在。\n",options->path);
        return -1;
    }
    prepare_work_dir(options);
    while(!feof(fp))
    {
        memset(buf,0,SIZE);
        memset(filename,0,SIZE);
        memset(text,0,SIZE);
        
        fgets(buf,SIZE,fp);
        if(buf[0] != '[')
            continue;
        //printf("buf=[%s]\n",buf);
        name_start = buf + 1;
        name_end = strstr(buf,name_split);
        if(name_end)
        {
            strncpy(filename,name_start,name_end-name_start);
            text_start = name_end + 1;
            text_end = strstr(text_start,text_split);
            if(text_end)
            {
                strncpy(text,text_start,text_end-text_start);
                options->filename = filename;
                options->text = text;
                //printf("filename:[%s],text:[%s]\n",filename,text);
                get_turing_tts_url(options);
                download_file_by_url(options);
            }
        }
    }
    fclose(fp);
    
    return 0;
}



int main(int argc,char **argv)
{
    tts_t options = {0};
    int ret = 0;
    
    params_init(&options);
    parse_cmdline(argc,argv,&options);
    ret = parse_text_from_file(&options);
    if(1 == ret)
    {
        prepare_work_dir(&options);
        get_turing_tts_url(&options);
        download_file_by_url(&options);
    }
    
    return 0;
}

