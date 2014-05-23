/*******************************************************
 * name	: player_rm.c
 * function: rm play relative funtions
 * date	: 2010.2.23
 *******************************************************/
#include <player.h>
#include "player_error.h"
#include "stream_decoder.h"
#include "log_print.h"
#include "player_av.h"

#define REAL_COOKINFO_SIZE (2044)
static int stream_rm_init(play_para_t *p_para)
{
    v_stream_info_t *vinfo=&p_para->vstream_info;
    a_stream_info_t *ainfo=&p_para->astream_info;
    ByteIOContext *pb = p_para->pFormatCtx->pb;
    codec_para_t *codec;   
    static unsigned short tbl[9];
    int i, j;        
    int rev_byte;

    codec=codec_alloc();
    if(!codec)
    {
        return PLAYER_EMPTY_P;
    }
    MEMSET(codec, 0, sizeof(codec_para_t));
    
    if (codec_init_cntl(codec) != 0)
        goto error1;

    if(vinfo->has_video)
    {
        codec->has_video = 1;
        codec->video_type = vinfo->video_format;
        codec->video_pid = vinfo->video_pid;

        /* set video sysinfo */
        codec->am_sysinfo.format = vinfo->video_codec_type;
        codec->am_sysinfo.width = vinfo->video_width;
        codec->am_sysinfo.height = vinfo->video_height;
        codec->am_sysinfo.rate = vinfo->video_codec_rate;
        codec->am_sysinfo.ratio = 0x100;
        if (VIDEO_DEC_FORMAT_REAL_8 == vinfo->video_codec_type)
        {
            codec->am_sysinfo.extra = vinfo->extradata[1]&7;
            tbl[0] = (((codec->am_sysinfo.width>>2)-1)<<8) | (((codec->am_sysinfo.height>>2)-1)&0xff);
            for (i=1; i<=codec->am_sysinfo.extra; i++)
            {
                j = 2*(i-1);
                tbl[i] = ((vinfo->extradata[8+j]-1)<<8) | ((vinfo->extradata[8+j+1]-1)&0xff);
            }
        }
        codec->am_sysinfo.param = &tbl;
        
        log_print("video_type = %d  video_pid = %d\n", codec->video_type, codec->video_pid);
    }
    
    if(ainfo->has_audio)
    {     
        static char extradata[REAL_COOKINFO_SIZE];
        codec->has_audio = 1;
        codec->audio_type=ainfo->audio_format;
        codec->audio_pid=ainfo->audio_pid;		
        codec->audio_channels = ainfo->audio_channel;
        codec->audio_samplerate = ainfo->audio_samplerate;       
        if(0 == p_para->playctrl_info.search_flag)
        {
            url_fseek(p_para->pFormatCtx->pb,0, SEEK_SET);
            rev_byte = get_buffer(pb,(void*)codec->audio_info.extradata, REAL_COOKINFO_SIZE);              
            memcpy(extradata,codec->audio_info.extradata,REAL_COOKINFO_SIZE);
        }
        else
            memcpy(codec->audio_info.extradata,extradata,REAL_COOKINFO_SIZE);
        codec->audio_info.valid = 1;
        log_print("audio_type = %d  audio_pid = %d channel= %d rate=%d\n", codec->audio_type, codec->audio_pid, codec->audio_channels,codec->audio_samplerate);        
    }    
    
    codec->stream_type= stream_type_convert(p_para->stream_type, codec->has_video, codec->has_audio);
    if(codec_init(codec) < 0)
    {
        goto error1;
    }

    p_para->codec = codec;   
    if (p_para->pFormatCtx->streams[vinfo->video_index]->stream_offset > 0)
        p_para->data_offset = p_para->pFormatCtx->streams[vinfo->video_index]->stream_offset;
    else
        p_para->data_offset = p_para->pFormatCtx->data_offset;

    return PLAYER_SUCCESS;

error1:
    log_print("[rm]codec_init failed!\n");   
    codec_free(codec);    
    return DECODER_INIT_FAILED;
}

static int stream_rm_release(play_para_t *p_para)
{
    if(p_para->codec)
    {
        codec_close_cntl(p_para->codec);
    	codec_close(p_para->codec);
        codec_free(p_para->codec);
    }    
    p_para->codec=NULL;

    return 0;
}

static const stream_decoder_t rm_decoder=
{
    .name="RM",
    .type=STREAM_RM,
    .init=stream_rm_init,
    .add_header=NULL,
    .release=stream_rm_release,
};

int rm_register_stream_decoder()
{
    return register_stream_decoder(&rm_decoder);
}

