#ifndef __WAV_H__
#define __WAV_H__

struct RIFF_HEADER_DEF
 {
  char riff_id[4];  // 'R','I','F','F'
  unsigned int  riff_size;
  char riff_format[4]; // 'W','A','V','E'
 };
 
 typedef struct _WAVE_FORMAT
{
   unsigned short FormatTag;
   unsigned short Channels;
   unsigned int SamplesPerSec;
   unsigned int AvgBytesPerSec;
   unsigned short BlockAlign;
   unsigned short BitsPerSample;
}WAVE_FORMAT;
 
 
 struct FMT_BLOCK_DEF
{
    char fmt_id[4];   // 'f','m','t',' ' please note the
    unsigned int fmt_size;
    WAVE_FORMAT wav_format;
};

struct DATA_BLOCK_DEF
{
   char data_id[4];   // 'd','a','t','a'
   unsigned int data_size;
};

typedef struct 
{
    int sample_rate;
    int channel;
    int sample_accuracy;
    unsigned int data_len;
}audio_para_t;


int set_wav_header(FILE *fp,audio_para_t *para);

int adjust_wav_header(char *wav_header,int wav_len,unsigned long data_len);

#endif
