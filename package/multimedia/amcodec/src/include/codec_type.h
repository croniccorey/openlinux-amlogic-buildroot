#ifndef CODEC_TYPE_H_
#define CODEC_TYPE_H_

#include "amports/amstream.h"
#include "amports/vformat.h"
#include "amports/aformat.h"

typedef int CODEC_HANDLE; 	

typedef enum
{
	STREAM_TYPE_UNKNOW,
	STREAM_TYPE_ES_VIDEO,
	STREAM_TYPE_ES_AUDIO,
	STREAM_TYPE_PS,
	STREAM_TYPE_TS,
	STREAM_TYPE_RM,
}stream_type;

typedef struct 
{
    unsigned int    format;
    unsigned int    width;
    unsigned int    height;
    unsigned int    rate;
    unsigned int    extra;
    unsigned int    status;
    unsigned int    ratio;
    void *          param;
} dec_sysinfo_t;

typedef struct  
{
    int valid;	
    int sample_rate;
    char extradata[2044];
}real_cookinfo_t;

typedef struct 
{
	CODEC_HANDLE handle;
    CODEC_HANDLE cntl_handle;
    CODEC_HANDLE sub_handle;
	stream_type stream_type;
	unsigned int has_video:1;
	unsigned int  has_audio:1;
    unsigned int has_sub:1;    
	int video_type;
	int audio_type;
	int video_pid;
	int audio_pid;
    int sub_pid;
	int audio_channels;
	int audio_samplerate;
	int vbuf_size;
	int abuf_size;
	dec_sysinfo_t am_sysinfo;
    real_cookinfo_t audio_info;
    int packet_size;
}codec_para_t;


#define IS_VALID_PID(t)		(t>=0 && t<=0x1fff)
#define IS_VALID_STREAM(t)	(t>0 && t<=0x1fff)
#define IS_VALID_ATYPE(t)	(t>=0 && t<AFORMAT_MAX)
#define IS_VALID_VTYPE(t)	(t>=0 && t<VFORMAT_MAX)

#endif
