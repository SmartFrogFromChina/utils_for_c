#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wav.h"

//1.传入音频的采样率、声道和采样精度的结构体
//2.传入数据长度
//3.传出头部大小
//4.返回wav头部指针
int  set_wav_header(FILE *fp,audio_para_t *para)
{   
    struct RIFF_HEADER_DEF riff_header;
    struct FMT_BLOCK_DEF fmt_block;
    struct DATA_BLOCK_DEF data_block;
    char *header = malloc(256);
    int header_len = 0;
    
    memset(&riff_header, 0, sizeof(riff_header));
    memset(&fmt_block, 0, sizeof(fmt_block));
    memset(&data_block, 0, sizeof(data_block));

    memcpy(&riff_header.riff_id[0], "RIFF", sizeof(riff_header.riff_id));
    memcpy(&riff_header.riff_format[0], "WAVE", sizeof(riff_header.riff_format));
    memcpy(&fmt_block.fmt_id[0], "fmt ", sizeof(riff_header.riff_id));
    fmt_block.fmt_size = 16;
    fmt_block.wav_format.FormatTag = 1;
    fmt_block.wav_format.Channels = para->channel;
    fmt_block.wav_format.SamplesPerSec = para->sample_rate;
    fmt_block.wav_format.AvgBytesPerSec = para->sample_rate * para->sample_accuracy/8;
    fmt_block.wav_format.BlockAlign = para->channel * para->sample_accuracy / 8;
    fmt_block.wav_format.BitsPerSample = para->sample_accuracy;

    memcpy(&data_block.data_id[0], "data", sizeof(data_block.data_id));
    data_block.data_size = para->data_len;

    header_len = sizeof(riff_header) + sizeof(fmt_block) + sizeof(data_block);
    riff_header.riff_size = header_len  + para->data_len - 8;

    header_len = 0;
    memcpy(&header[header_len], &riff_header, sizeof(riff_header));
    header_len += sizeof(riff_header);
    memcpy(&header[header_len], &fmt_block, sizeof(fmt_block));
    header_len += sizeof(fmt_block);
    memcpy(&header[header_len], &data_block, sizeof(data_block));
    header_len += sizeof(data_block);
    
    fwrite(header, 1, header_len, fp);
    free(header);
    
    return header_len;
}

int adjust_wav_header(char *wav_header,int wav_len,unsigned long data_len)
{
    char *riff_size = wav_header + 4;
    *(unsigned int  *)riff_size = wav_len + data_len -8;

    char *data_size = wav_header + wav_len -4;
    *(unsigned int  *)data_size = data_len;

    return 0;
}

