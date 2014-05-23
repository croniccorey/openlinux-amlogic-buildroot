/*****************************************
 * name	: player_es.c
 * function: es player relative functions
 * date		: 2010.2.4
 *****************************************/
#include <codec.h>
#include "player_hwdec.h"
#include "player_error.h"
#include "player_priv.h"
#include "stream_decoder.h"
#include "player_es.h"

static int stream_es_init(play_para_t *p_para)
{
	v_stream_info_t *vinfo= &p_para->vstream_info;
	a_stream_info_t *ainfo= &p_para->astream_info;
	codec_para_t *v_codec, *a_codec; 		
		
	if(vinfo->has_video)
	{        
		v_codec= codec_alloc();
		if(!v_codec)
			return PLAYER_EMPTY_P;
		MEMSET(v_codec, 0, sizeof(codec_para_t));
        if (codec_init_cntl(v_codec) != 0)
        {
            codec_free(v_codec);
            return DECODER_INIT_FAILED;
        }

		v_codec->has_video = 1;
		v_codec->video_type 		= vinfo->video_format;
		v_codec->video_pid 			= vinfo->video_pid;		
		v_codec->am_sysinfo.format 	= vinfo->video_codec_type;
		v_codec->am_sysinfo.height 	= vinfo->video_height;
		v_codec->am_sysinfo.width 	= vinfo->video_width;
		v_codec->am_sysinfo.rate 		= vinfo->video_rate;
		v_codec->am_sysinfo.ratio		= vinfo->video_ratio;
        if ((vinfo->video_format == VFORMAT_MPEG4) 
            || (vinfo->video_format == VFORMAT_H264))
		    v_codec->am_sysinfo.param	    = (void *)EXTERNAL_PTS;
        else
            v_codec->am_sysinfo.param	    = (void *)0;
		v_codec->stream_type        = STREAM_TYPE_ES_VIDEO;			
		v_codec->stream_type        = stream_type_convert(p_para->stream_type, v_codec->has_video, 0);	
       
		if(codec_init(v_codec) != 0)		
			return DECODER_INIT_FAILED;
		p_para->vcodec = v_codec;        
	}	

	if(ainfo->has_audio)
	{        
		a_codec=codec_alloc();
		if(!a_codec)
			return PLAYER_EMPTY_P;

		MEMSET(a_codec, 0, sizeof(codec_para_t));
        if (codec_init_cntl(a_codec) != 0)
        {
            codec_free(a_codec);
            return DECODER_INIT_FAILED;
        }

		a_codec->has_audio = 1;
		a_codec->audio_type=ainfo->audio_format;
		a_codec->audio_pid=ainfo->audio_pid;		
		a_codec->audio_channels = ainfo->audio_channel;
		a_codec->audio_samplerate = ainfo->audio_samplerate;	
		a_codec->stream_type = STREAM_TYPE_ES_AUDIO;		
		a_codec->stream_type= stream_type_convert(p_para->stream_type, 0, a_codec->has_audio);	
        
		if(codec_init(a_codec) != 0)			
		{
            return DECODER_INIT_FAILED;
		}	    
		p_para->acodec = a_codec;
	}    
    if(!p_para->vcodec && !p_para->acodec)
    {
        log_print("[stream_es_init] no audio and no video codec init!\n");
        return DECODER_INIT_FAILED;
    }
	return PLAYER_SUCCESS;
}
static int stream_es_release(play_para_t *p_para)
{
    int r = -1;
	if(p_para->acodec)
	{
    	r = codec_close(p_para->acodec);
        if(r < 0)
        {
            log_error("[stream_es_release]close acodec failed, r= %x\n",r);
            return r;
        }
    	codec_free(p_para->acodec);
        p_para->acodec=NULL;
	}
	if(p_para->vcodec)
	{
	    r = codec_close_cntl(p_para->vcodec);
        if(r < 0)
        {
            log_error("[stream_es_release]close vcodec control handle failed, r= %x\n",r);
            return r;
        }
    	r = codec_close(p_para->vcodec);
        if(r < 0)
        {
            log_error("[stream_es_release]close vcodec failed, r= %x\n",r);
            return r;
        }
    	codec_free(p_para->vcodec);
        p_para->vcodec=NULL;
	}
	return 0;
}

static const stream_decoder_t es_decoder=
{
	.name="ES",
	.type=STREAM_ES,
	.init=stream_es_init,
	.add_header=NULL,
	.release=stream_es_release,
};

int es_register_stream_decoder()
{
	return register_stream_decoder(&es_decoder);
}
