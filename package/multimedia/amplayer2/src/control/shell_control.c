#include <stdio.h>
#include <codec.h>
#include <player_ctrl.h>
#include "log_print.h"
#include "controler.h"
#include "player_id.h"
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

void shell_usage(void)
{
	log_print("\n-----------Player Control Command:----------------\n");
	log_print("\tx:quit\n");
	log_print("\ts:stop\n");
	log_print("\tp:pause\n");
	log_print("\tr:resume\n");
    log_print("\tt:search\n");
    log_print("\tf:forward, fast forward\n");
    log_print("\tb:backward, fast backward\n");
	log_print("\tm:mute\n");
	log_print("\tM:unmute\n");
	log_print("\tv:volget,get currnet sound volume\n");
	log_print("\tV:volset(V:val),set currnet sound volume\n");
    log_print("\ta:aid, set new audio id to play\n");
    log_print("\tu:spectrum, audio switch spectrum\n");
    log_print("\tw:swap, swap audio left and right\n");
    log_print("\tl:lmono, audio set left mono\n");
    log_print("\tL:rmono, audio set right mono\n");
    log_print("\to:stereo, audio set stereo\n");
    log_print("\tc:curtime, get current playing time\n");
    log_print("\td:duration, get stream duration\n");
    log_print("\tS:status, get player status\n");
    log_print("\ti:media, get media infomation\n");   
    log_print("\tk:pid, list all valid player id\n");   
	log_print("\th:help\n");
}

static int shell_get_command(player_cmd_t *playercmd)
{

	int  ret;
    fd_set rfds;
    struct timeval tv;
	int fd=fileno(stdin);
    
#define is_CMD(str,cmd,s_cmd1,s_cmd2) (strcmp(str,cmd)==0 || strcmp(str,s_cmd1)==0 || strcmp(str,s_cmd2)==0)

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 3 * 1000 * 1000;       
	ret = select(fd+1, &rfds, NULL, NULL, &tv);
   
	if(ret > 0 && FD_ISSET(fd, &rfds))
	{            
    	char str[100]={0};
    	scanf("%s",str);
    	//gets(str);
    	if(is_CMD(str,"quit","x","X"))
		{
		    playercmd->ctrl_cmd = CMD_EXIT;
		}
	    else if(is_CMD(str,"stop","s","s"))
		{
		    playercmd->ctrl_cmd = CMD_STOP;
		}
	    else if(is_CMD(str,"pause","p","P"))
		{
		    playercmd->ctrl_cmd = CMD_PAUSE;              
		}
	    else if(is_CMD(str,"resume","r","R"))
		{
		    playercmd->ctrl_cmd = CMD_RESUME;
		}
	    else if(is_CMD(str,"mute","m","m"))
		{			
            playercmd->set_mode = CMD_MUTE ;
		}
	    else if(is_CMD(str,"unmute","M","M"))
		{		
            playercmd->set_mode = CMD_UNMUTE;
		}
	    else if(is_CMD(str,"volget","v","v"))
		{
            #if 1
            playercmd->info_cmd = CMD_GET_VOLUME;
            #else
    		int r;
    		r=audio_get_volume(0);
    		if(r>=0)
    			log_print("Volume=%d\n",r);
    		else
    			log_print("Volume get failed=%d\n",r);
			#endif
		}
	    else if(memcmp(str,"volset:",strlen("volset:"))==0 ||
			(str[0]=='V' && str[1]==':'))
    	{            
    		int r=-1;
    		char *p;
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);               
                #if 1
                playercmd->set_mode = CMD_SET_VOLUME;
                playercmd->param = r;
                #else
    			r=audio_set_volume(0,r);
                #endif
			}
    		else
			{
			    log_print("error command for set vol(fmt:\"V:value\")\n");
			}           
    	}
        else if(memcmp(str,"search:",strlen("search:"))==0 ||
			(str[0]=='t' && str[1]==':')||
			(str[0]=='T' && str[1]==':'))
		{            
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                
                playercmd->ctrl_cmd = CMD_SEARCH;
                playercmd->param = r;                
			}
    		else
			{
			    log_print("error command for search(fmt:\"T:value\")\n");
			}            
		}
        else if(memcmp(str,"forward:",strlen("forward:"))==0 ||
			(str[0]=='f' && str[1]==':')||
			(str[0]=='F' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                 
                playercmd->ctrl_cmd = CMD_FF;
                playercmd->param= r;               
			}
    		else
			{
			    log_print("error command for forward(fmt:\"F:value\")\n");
			}
		}
        else if(memcmp(str,"backward:",strlen("backward:"))==0 ||
			(str[0]=='b' && str[1]==':')||
			(str[0]=='B' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                  
                playercmd->ctrl_cmd = CMD_FB;
                playercmd->param= r;    
                printf("[%s:%d]fb---step=%d\n",__FUNCTION__, __LINE__, r);
			}
    		else
			{
			    log_print("error command for backward(fmt:\"B:value\")\n");
			}
		}
        else if(memcmp(str,"aid:",strlen("aid:"))==0 ||
			(str[0]=='a' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                  
                playercmd->ctrl_cmd = CMD_SWITCH_AID;
                playercmd->param= r;               
			}
    		else
			{
			    log_print("error command for audio id(fmt:\"B:value\")\n");
			}
		}
        else if(is_CMD(str, "volrange", "g", "G"))
        {
            playercmd->info_cmd = CMD_GET_VOL_RANGE;
        }
        else if(is_CMD(str, "spectrum", "u", "U"))
        {
            playercmd->set_mode = CMD_SPECTRUM_SWITCH;
        }
        else if(is_CMD(str, "balance", "e", "E"))
        {
            playercmd->set_mode = CMD_SET_BALANCE;
        }
        else if(is_CMD(str, "swap", "l", "L"))
        {
            playercmd->set_mode = CMD_SWAP_LR;
        }       
        else if(is_CMD(str, "lmono", "l", "l"))
        {
            playercmd->set_mode = CMD_LEFT_MONO;
        }
        else if(is_CMD(str, "rmono", "L", "L"))
        {
            playercmd->set_mode = CMD_RIGHT_MONO;
        }
        else if(is_CMD(str, "setreo", "o", "O"))
        {
            playercmd->set_mode = CMD_SET_STEREO;
        }
        else if(is_CMD(str, "curtime", "c", "C"))
        {
            playercmd->info_cmd = CMD_GET_CURTIME;
        }
        else if(is_CMD(str, "duration", "d", "D"))
        {
            playercmd->info_cmd = CMD_GET_DURATION;
        }
        else if(is_CMD(str, "status", "S", "S"))
        {
            playercmd->info_cmd = CMD_GET_PLAY_STA;
        }
        else if(is_CMD(str, "media", "i", "I"))
        {
            playercmd->info_cmd = CMD_GET_MEDIA_INFO;
        }
        else if(is_CMD(str, "pid", "k", "K"))
        {
            playercmd->info_cmd = CMD_LIST_PID;
        }
    	else if(is_CMD(str,"help","h","H"))
    	{
    	    shell_usage();
    	}
    	else
    	{
        	if(strlen(str) > 1) // not only enter
        	    shell_usage();
    	}
	}  
    else if(ret < 0)
        log_print("[shell_get_command] select error!\n");
    //else    //ret = 0
        //log_print("[shell_get_command] select time out!\n");
    //printf("[shell_get_command:%d]ccmd=%x param=%d\n",__LINE__,playercmd->ctrl_cmd,playercmd->param);    
	if((playercmd->ctrl_cmd!=0) || 
        (playercmd->info_cmd!=0)|| 
        (playercmd->set_mode != 0))
    {
       
        char pid[MAX_PLAYER_THREADS];
        int num = 0;
        num = player_list_pid(pid, MAX_PLAYER_THREADS);
        if(num == 1)
           playercmd->pid = pid[0];
        //printf("[shell_get_command:%d]num=%d pid=%d param=%d\n",__LINE__,num,playercmd->pid, playercmd->param);
        return 0;
	}
    else
		return -1;
}
static int shell_update_state(int pid,player_info_t *player_info)
{
	#if 0
	log_print("Playing %s(%%%2d.%02d(%d,%d))(%d,%d)\r",  
		                                          player_info->name,
								player_info->current_time*100/(player_info->full_time+1),
								(player_info->current_time*10000/(player_info->full_time+1)%100),
								player_info->current_time,
								player_info->full_time,
								player_info->video_error_cnt,
								player_info->audio_error_cnt);
	#else
	log_print("[PID=%d]Playing %s-%2d.%02d%%-%02d:%02d:%02d(%02d:%02d:%02d)(%d,%d)\r", 
								pid,
		                        player_info->name,
								player_info->current_time*100/(player_info->full_time+1),
								(player_info->current_time*10000/(player_info->full_time+1)%100),
								player_info->current_time/3600,
							    (player_info->current_time/60)%60,
								player_info->current_time%60,
								player_info->full_time/3600,
								(player_info->full_time/60)%60,
								player_info->full_time%60,
								player_info->video_error_cnt,
								player_info->audio_error_cnt);    
	#endif
	return 0;
}

static int shell_return_info(int pid, int cid, int type, void *data)
{    
    if(type == CMD_GET_VOL_RANGE)
    {
        volume_range_t *range = (volume_range_t *)data;
        log_print("[shell_return_info]pid=%d cid=%d type=%d min=%d max=%d\n", pid, cid, type,range->min,range->max);
    }
    else if(type == CMD_LIST_PID)
    {
        int i;
        pid_info_t* playerid = (pid_info_t *)data;
        log_print("[shell_return_info]pid=%d cid=%d type=%d num=%d\n", pid, cid, type,playerid->num);
        log_print("valid pid:");
        for(i = 0; i < playerid->num; i++)
            log_print("%d",playerid->pid[i]);
        log_print("\n");
    }
    else if(type == CMD_GET_MEDIA_INFO)
    {
        int i;
        media_info_t *minfo = (media_info_t *)data;          
        printf("\n***************************************\n");
        printf("stream info:\n");
        printf("file size:      %lld\n",minfo->stream_info.file_size);
        printf("file duration:  %d(s)\n",minfo->stream_info.duration);
        printf("has video:      %d\n",minfo->stream_info.has_video);
        printf("has audio:      %d\n",minfo->stream_info.has_audio);
        printf("has subtitle:   %d\n",minfo->stream_info.has_sub);
        printf("stream bitrate: %d\n",minfo->stream_info.bitrate);
        printf("video streams:  %d\n",minfo->stream_info.total_video_num);
        printf("audio streams:  %d\n",minfo->stream_info.total_audio_num);
        printf("sub streams:    %d\n",minfo->stream_info.total_sub_num);
        printf("\n");
        for(i=0; i<minfo->stream_info.total_video_num;i++)    
        {
            printf("video id:       %d\n",minfo->video_info[i]->id);
            printf("video width:    %d\n",minfo->video_info[i]->width);
            printf("video height:   %d\n",minfo->video_info[i]->height);
            printf("video ratio:    %d:%d\n",minfo->video_info[i]->aspect_ratio_num,minfo->video_info[i]->aspect_ratio_den);
            printf("frame_rate:     %.2f\n",(float)minfo->video_info[i]->frame_rate_num/minfo->video_info[i]->frame_rate_den);
            printf("video bitrate:  %d\n",minfo->video_info[i]->bit_rate);
            printf("video format:   %d\n",minfo->video_info[i]->format);
            printf("video duration: %d\n",minfo->video_info[i]->duartion);
            printf("----------------------\n");
        }
        for(i=0; i<minfo->stream_info.total_audio_num;i++)    
        {
            printf("audio id:       %d\n",minfo->audio_info[i]->id);            
            printf("audio duration: %d\n",minfo->audio_info[i]->duration);
            printf("audio channel:  %d\n",minfo->audio_info[i]->channel);
            printf("sample rate:    %d\n",minfo->audio_info[i]->sample_rate);
            printf("audio format:   %d\n",minfo->audio_info[i]->aformat);
            printf("audio bitrate:  %d\n",minfo->audio_info[i]->bit_rate);
            if(minfo->audio_info[i]->audio_tag)
            {
                printf("title:       %s\n",minfo->audio_info[i]->audio_tag->title);  
                printf("author:      %s\n",minfo->audio_info[i]->audio_tag->author);
                printf("comment:     %s\n",minfo->audio_info[i]->audio_tag->album);
                printf("album:       %s\n",minfo->audio_info[i]->audio_tag->comment);
                printf("year:        %s\n",minfo->audio_info[i]->audio_tag->year);
                printf("track:       %d\n",minfo->audio_info[i]->audio_tag->track);
                printf("genre:       %s\n",minfo->audio_info[i]->audio_tag->genre);
                printf("copyright:   %s\n",minfo->audio_info[i]->audio_tag->copyright);
                printf("genre:       %c\n",minfo->audio_info[i]->audio_tag->pic);
            }
            printf("-----------------------\n");
        }
        for(i=0; i<minfo->stream_info.total_sub_num;i++)    
        {
            printf("sub id:       %d\n",minfo->sub_info[i]->id);
            printf("inter_ex:     %d\n",minfo->sub_info[i]->internal_external);
            printf("sub width:    %d\n",minfo->sub_info[i]->width);
            printf("sub height:   %d\n",minfo->sub_info[i]->height);
            printf("resolution:   %d\n",minfo->sub_info[i]->resolution);
            printf("sub size:     %lld\n",minfo->sub_info[i]->subtitle_size);
            printf("language:     %d\n",minfo->sub_info[i]->sub_language);
        }
        printf("***************************************\n");   
    }
    else
    {
        int *val = (int *)data;
        log_print("[shell_return_info]pid=%d cid=%d type=%d data=%d\n", pid, cid, type,*val);    
    }
    
    return 0;
}

player_controler_t shell_cmd=
{
	.name="shell",
	.front_mode=1,
	.get_command=shell_get_command,
	.update_state=shell_update_state,
	.ret_info=shell_return_info,
};

int register_shell_controler(void)
{	
	register_controler(&shell_cmd);
	return 0;
}

