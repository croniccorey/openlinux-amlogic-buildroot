#ifndef _PLAYER_AV_H_
#define _PLAYER_AV_H_

#include "player_priv.h"
#include "player_para.h"
struct play_para;

typedef int CODEC_TYPE;
#define CODEC_UNKNOW		(0)
#define CODEC_VIDEO			(1)
#define CODEC_AUDIO  		(2)
#define CODEC_COMPLEX  		(3)
#define CODEC_SUBTITLE        (4)

#define RW_WAIT_TIME        (20 * 1000) //20ms

typedef struct hdr_buf
{	
	char *data;
	int size;
}hdr_buf_t;

typedef struct am_packet
{
	CODEC_TYPE type;
	AVPacket *avpkt;
	int avpkt_isvalid;
    int avpkt_newflag;    
	unsigned char *data;
	unsigned char *buf;
	int data_size;
    int buf_size;
	hdr_buf_t *hdr;
	codec_para_t *codec;	
}am_packet_t;

aformat_t audio_type_convert(enum CodecID id,pfile_type File_type);
vformat_t video_type_convert(enum CodecID id);
vdec_type_t video_codec_type_convert(unsigned int id);
stream_type_t stream_type_convert(pstream_type type, char vflag, char aflag);
void av_packet_init(struct am_packet *pkt);
int set_file_type(const char *name, pfile_type *ftype, pstream_type *stype);
int read_av_packet(play_para_t *para, am_packet_t *pkt);
int write_av_packet(play_para_t *para,am_packet_t *pkt);
int check_in_pts(play_para_t *para, am_packet_t *pkt);
int set_header_info(play_para_t *para, am_packet_t *pkt);
void av_packet_release(am_packet_t *pkt);
int poll_sub(am_packet_t *pkt);
int get_sub_size(am_packet_t *pkt);
int read_sub_data(am_packet_t *pkt, char *buf, unsigned int length);
int write_sub_data(am_packet_t *pkt, char *buf, unsigned int length);
int process_es_subtitle(play_para_t *para, am_packet_t *pkt);
int poll_cntl(am_packet_t *pkt);
int set_cntl_mode(play_para_t *para, unsigned int mode);
int set_cntl_avthresh(play_para_t *para, unsigned int avthresh);
int set_cntl_syncthresh(play_para_t *para);
void player_switch_audio(play_para_t *para);
void player_switch_sub(play_para_t *para);
int get_cntl_state(am_packet_t *pkt);
int time_search(struct play_para *para);
int player_reset(play_para_t *p_para,am_packet_t *pkt);

#endif
