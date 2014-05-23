#include <stdio.h>
#include <string.h>
#include <codec.h>
#include <fcntl.h>
#include <string.h>

#include <player.h>
#include <player_ctrl.h>
#include <player_id.h>
#include <log_print.h>
#include <player_set_sys.h>
#include <controler.h>
#include <unistd.h>

#include "shell_control.h"
#include "socket_controler.h"


#define MAX_CONTROLER 16
#define VALID_CMD       (0)
#define INVALID_CMD     (-100)
static  player_controler_t *control_list[MAX_CONTROLER];
static int control_index=0;
int update_statue(int pid,player_info_t *);


int start_controler(global_ctrl_para_t *player_para)
{
	int i;
	player_controler_t *controler=NULL;
	if(player_para->control_mode[0]=='\0') /*not set*/
		strcpy(player_para->control_mode,"shell");
	for(i=0;i<control_index;i++)
		{          
		if(IS_MODE(player_para->control_mode,control_list[i]->name))
			{
			controler=control_list[i];
			break;
			}
		}
	if(!controler)
		{
		log_print("can't find the controler %s\n",player_para->control_mode);
		return -1;
		}
	if(controler->front_mode)
		player_para->background=0;
	player_para->controler=controler;
	
	return 0;
}

int register_controler(player_controler_t *controler)
{
	if(control_index<MAX_CONTROLER)
		{
		control_list[control_index++]=controler;
		}
	else
		{
		log_print("register_controler %s failed,too many controlers,num=%d\n",controler->name,control_index);
		return -1;
		}
	return 0;
}


void basic_controlers_init(void)
{
	control_index=0;
	MEMSET(control_list,0,sizeof(control_list));
	register_shell_controler();
	register_socket_controler();
}

int get_command(global_ctrl_para_t *p,player_cmd_t *playercmd)
{
	if(!playercmd)
		return -1;
	return p->controler->get_command(playercmd);
}

int update_statue(int pid,player_info_t *player_state)
{
	struct control_para *control_para;
	player_controler_t *ctrl;
	if(!player_state)
		return -1;
	control_para=(void *)player_get_extern_priv(pid);
	if(control_para!=NULL)
	{
    	ctrl=control_para->controler;  
    	if(NULL!=ctrl->update_state)        
            return ctrl->update_state(pid,player_state);      	
        else           
            return 0;            
	}
	else	
        return -1;	
}
static void clear_player_ctrl_para(int pid, global_ctrl_para_t *p)
{
    play_control_t *para = &p->g_play_ctrl_para;    
    if(para)
    {
        para->file_name = NULL;
        para->audio_index = -1;
        para->video_index = -1;
        para->loop_mode = 0;
        para->nosound = 0;
        para->novideo = 0;    
    }
}

static int is_valid_command(int pid,player_cmd_t *cmd)
{
    player_status sta;
    if(check_pid_valid(pid))
    {
        sta = player_get_state(pid);
        printf("[is_valid_command:%d]pid=%d sta=%d\n", __LINE__, pid, sta);
        if(sta == PLAYER_STOPED || sta == PLAYER_ERROR || sta == PLAYER_PLAYEND)
        {
            printf("[is_valid_command:%d]cmd->ctrl_cmd=%x (%x)\n", __LINE__, cmd->ctrl_cmd, (CMD_STOP | CMD_SEARCH | CMD_FB | CMD_FF | CMD_SWITCH_AID));
            if(cmd->ctrl_cmd & (CMD_STOP | CMD_SEARCH | CMD_FB | CMD_FF | CMD_SWITCH_AID))
            {            
                return INVALID_CMD;
            }
        }        
    } 
    if(cmd->ctrl_cmd != 0 || cmd->set_mode != 0 || cmd->info_cmd != 0)
        return VALID_CMD;
    else
        return INVALID_CMD;
}

static int parse_command(global_ctrl_para_t *p, player_cmd_t *cmd)
{    
    int ret = 0;    
	int pid = cmd->pid;	 
    //printf("[parse_command:%d]pid=%d ctrl_cmd=%x set_mode=%x info=%x\n", __LINE__,pid, cmd->ctrl_cmd, cmd->set_mode,cmd->info_cmd);
    ret = is_valid_command(pid,cmd);
    if(ret == VALID_CMD)
    {
        if(cmd->set_mode != 0)
            printf("[parse_command:%d]pid=%d set_mode=%x param=%d %x\n",__LINE__,pid,cmd->set_mode, cmd->param,cmd->param1);
        //set play mode
        if(cmd->set_mode & CMD_LOOP)
        {               
            if(check_pid_valid(pid))
            {
                player_loop(pid);
                p->thread_ctrl_para[pid]->play_ctrl_para.loop_mode = 1;
            }
            else
                p->g_play_ctrl_para.loop_mode = 1;
        }
        else if(cmd->set_mode & CMD_NOLOOP)
        {
            if(check_pid_valid(pid))
            {
                player_noloop(pid);
                p->thread_ctrl_para[pid]->play_ctrl_para.loop_mode = 0;
            }
            else
                p->g_play_ctrl_para.loop_mode = 0;
        }

        if(cmd->set_mode & CMD_NOBLACK)
        {
            set_black_policy(0);        
        }
        else if(cmd->set_mode & CMD_BLACKOUT)
        {
            set_black_policy(1);        
        }

        if(cmd->set_mode & CMD_NOAUDIO)
        {
            p->g_play_ctrl_para.nosound = 1;
        }    
        
        if(cmd->set_mode & CMD_NOVIDEO)
        {
            p->g_play_ctrl_para.novideo = 1;
        } 

        if(cmd->set_mode & CMD_MUTE)
        {
            audio_set_mute(pid, 1);
        }   
        else if(cmd->set_mode & CMD_UNMUTE)
        {
            audio_set_mute(pid, 0);
        }
        if(cmd->set_mode & CMD_SET_VOLUME)
        {
            int r = -1;           
            int val = cmd->param;
            log_print("[CMD_SET_VOLUME]val=%d\n", val);
            r = audio_set_volume(pid, val);     
            if(r < 0)                
                log_print("[CMD_SET_VOLUME]failed! r=%x\n", -r);
        }
          if(cmd->set_mode & CMD_SPECTRUM_SWITCH)
        {
            int r = -1;           
            int isStart = cmd->param;
            int interval= cmd->param1;
            r = audio_set_spectrum_switch(pid, isStart, interval);
            if(r < 0)                
                log_print("[CMD_SPECTRUM_SWITCH]r=%x\n", r);
        }
        if(cmd->set_mode & CMD_SET_BALANCE)
        {
            int r = -1;            
            int balance = cmd->param;
            r = audio_set_volume_balance(pid, balance);
            if(r < 0)                
                log_print("[CMD_SET_BALANCE]r=%x\n", r);
        }
        if(cmd->set_mode & CMD_SWAP_LR)
        {
            int r = -1;            
            r = audio_swap_left_right(pid);
            if(r < 0)                
                log_print("[CMD_SWAP_LR]r=%x\n", r);
        }
        if(cmd->set_mode & CMD_LEFT_MONO)
        {
            int r = -1;            
            r = audio_left_mono(pid);
            if(r < 0)                
                log_print("[CMD_LEFT_MONO]r=%x\n", r);
        }
         if(cmd->set_mode & CMD_RIGHT_MONO)
        {
            int r = -1;            
            r = audio_left_mono(pid);
            if(r < 0)                
                log_print("[CMD_RIGHT_MONO]r=%x\n", r);
        }
        if(cmd->set_mode & CMD_SET_STEREO)
        {
            int r = -1;           
            r = audio_stereo(pid);
            if(r < 0)                
                log_print("[CMD_SET_STEREO]r=%x\n", r);
        }
        if(cmd->info_cmd != 0)
            printf("[parse_command:%d]info_cmd=%x\n",__LINE__,cmd->info_cmd);
        //get information
        if(cmd->info_cmd & CMD_GET_VOLUME)
        {
            int r = -1;
            r = audio_get_volume(pid);
            log_print("get audio_volume=%d\n", r);
            if(r >= 0)
            {
                log_print("get audio_volume=%d\n", r);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_VOLUME, &r);         
            }
            else
                log_print("[CMD_GET_VOLUME]r=%x\n",r);
        }
        if(cmd->info_cmd & CMD_GET_PLAY_STA)
        {
            int r = -1;
            r = player_get_state(pid);
            log_print("get player status=%d\n", r);
            if(r >= 0)
            {
                log_print("get player status=%d\n", r);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_PLAY_STA, &r);   
            }
            else
                log_print("[CMD_GET_PLAY_STA]r=%x\n", r);
        }
        if(cmd->info_cmd & CMD_GET_VOL_RANGE)
        {
            int r = -1;
            volume_range_t range;
            r = audio_get_volume_range(pid, &range.min, &range.max);
            if(r >= 0)
            {
                log_debug("get volume rang:%d~%d\n",range.min,range.max);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_VOL_RANGE, &range);   
            }
            else
                log_print("[CMD_GET_VOL_RANGE]r=%x\n", r);
        }

        if(cmd->info_cmd & CMD_GET_CURTIME)
        {
            int r = -1;
            player_info_t info;
            r = player_get_play_info(pid, &info);
            if(r>=0)
            {
                r = info.current_time;
                printf("get current time=%d\n", r);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_CURTIME, &r);    
            }
            else
                log_print("[CMD_GET_CURTIME]r=%x\n", r);
        }
        if(cmd->info_cmd & CMD_GET_DURATION)
        {
            int r = -1;
            player_info_t info;
            r = player_get_play_info(pid, &info);
            if(r >= 0)
            {
                r=info.full_time;
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_DURATION, &r);            
            }
            else
                log_print("[CMD_GET_DURATION]r=%x\n", r);
        }  
        if(cmd->info_cmd & CMD_GET_MEDIA_INFO)
        {
            int r = -1;
            media_info_t minfo;
            r = player_get_media_info(pid, &minfo);
            log_debug("get medai_info=%d\n", r);
            if(p->controler->ret_info)
                p->controler->ret_info(pid, cmd->cid, CMD_GET_MEDIA_INFO, &minfo);  
        }
        if(cmd->info_cmd & CMD_LIST_PID)
        {
            int r = -1;
            pid_info_t p_id;
            r = player_list_allpid(&p_id); 
            if(p->controler->ret_info)
                p->controler->ret_info(pid, cmd->cid, CMD_LIST_PID, &p_id);  
        }
        
        
        //control command
        if(cmd->ctrl_cmd > 0)
            printf("[parse_command:%d]cmd=%x param=%d\n",__LINE__,cmd->ctrl_cmd,cmd->param);
        
        if(cmd->ctrl_cmd & CMD_EXIT)
    	{
    		player_stop(pid);
            clear_player_ctrl_para(pid, p);
    		ret = -1;
    	}
    	else if(cmd->ctrl_cmd & CMD_STOP)
    	{     
    	    player_stop(pid);
            clear_player_ctrl_para(pid, p);
            ret = 1;
    	}
        else if(cmd->ctrl_cmd & CMD_PLAY || cmd->ctrl_cmd & CMD_PLAY_START)
        {
            thread_ctrl_para_t *t_para;  
            int ret = -1;
            pid_info_t player_id;           
            if(cmd->ctrl_cmd & CMD_PLAY_START)
            {
                player_status status;
                pid_info_t player_id;
                player_list_allpid(&player_id);
                int i;
                int valid_pid;
                for(i = 0; i < player_id.num; i ++)
                {
                    valid_pid = player_id.pid[i];
                    status = player_get_state(valid_pid);                
                    if((status != PLAYER_STOPED)&&(status!= PLAYER_ERROR))
            		{                    
                        player_stop(valid_pid);                         
                        do
                        {
                            status = player_get_state(valid_pid);
                            if((status == PLAYER_STOPED) || (status== PLAYER_ERROR))
                            {                               
                                destruct_thread_para(p->thread_ctrl_para[valid_pid]); 
								player_release(valid_pid);
                                player_release_pid(valid_pid);
                                break;
                            }
							player_thread_wait_ms(valid_pid, 100);
                        }while(1);                     
                        clear_player_ctrl_para(valid_pid, p);
            		}
                }
            }
            t_para = MALLOC(sizeof(thread_ctrl_para_t));
            construct_thread_para(t_para, &p->g_play_ctrl_para);           
    		
            if(cmd->ctrl_cmd & CMD_PLAY)
                t_para->play_ctrl_para.need_start = 1;
    		if((NULL == t_para->play_ctrl_para.file_name) && (NULL!=cmd->filename))    		
                t_para->play_ctrl_para.file_name = MALLOC(strlen(cmd->filename));
            
    		strcpy(t_para->play_ctrl_para.file_name,cmd->filename);
            printf("[parse_command:%d]filename=%s\n",__LINE__,t_para->play_ctrl_para.file_name);

			ret=player_start(t_para,(unsigned long)p); 
            if(ret < 0)
            {
                log_error("[parse_command]start player failed, %x\n", ret);
                return ret;
            }
            if(p->controler->ret_info)            
                p->controler->ret_info(t_para->pid, cmd->cid, cmd->ctrl_cmd, NULL);             
             
            printf("[%s:%d]player started! pid=%d \n",__FUNCTION__, __LINE__, t_para->pid);
		    ret = player_list_allpid(&player_id);
            p->thread_ctrl_para[player_id.num-1] = t_para;  
            
        	if(player_register_update_callback(t_para->pid,&update_statue,1000)!=PLAYER_SUCCESS)
                log_error("[parse_command:%d]player_register_update_callback failed!\n", __LINE__);       	
		}
        else if(cmd->ctrl_cmd & CMD_START)
	    {
	        player_send_message(pid, cmd);
	    }
		else if(cmd->ctrl_cmd & CMD_PAUSE)
	    {
	        player_pause(pid);
	    }
	    else if(cmd->ctrl_cmd & CMD_RESUME)
	    {
	        player_resume(pid);
	    }    
	    else if(cmd->ctrl_cmd & CMD_SEARCH)
	    {
            int s_time = cmd->param;
	        player_timesearch(pid,s_time);
	    }
	    else if(cmd->ctrl_cmd & CMD_FF)
	    {        
            int speed = cmd->param;
	        player_forward(pid,speed);
	    }
	    else if(cmd->ctrl_cmd & CMD_FB)
	    {        
            int speed = cmd->param;
	        player_backward(pid,speed);
	    }     
	    else if(cmd->ctrl_cmd & CMD_SWITCH_AID)
	    {
            int audio_id = cmd->param;
	        player_aid(pid,audio_id);
	    } 	   
	}   
    return ret;
}

void controler_run(global_ctrl_para_t *p)
{
    player_controler_t *ctrl=p->controler;    
    int ret=0;  
    player_status pstatus;
    pid_info_t player_id;
    
    int i;
    int pid;
    thread_ctrl_para_t *t_para; 
    
    if(ctrl->init)
    {
    	ret=ctrl->init(p);
    	if(ret<0)
    	{
    		log_print("[controler_run:%d]control init failed,%x\n",__LINE__,ret);
    		return ;
    	}		
    }
    if(p->g_play_ctrl_para.file_name!=NULL)
    {    
        t_para = MALLOC(sizeof(thread_ctrl_para_t));       
        construct_thread_para(t_para, &p->g_play_ctrl_para);      
        ret=player_start(t_para,(unsigned long)p); 
        if(ret < 0)
        {
            log_error("[controler_run:%d]start player failed, %x\n",__LINE__,ret);
            return;
        }          
        p->thread_ctrl_para[ret] = t_para;      

        if(player_register_update_callback(t_para->pid,&update_statue,1000)!=PLAYER_SUCCESS)
                log_error("[controler_run:%d]player_register_update_callback failed!\n", __LINE__);            
    }
    
	do
    {      
		player_cmd_t cmd;
		MEMSET(&cmd, 0, sizeof(player_cmd_t));
		ret = get_command(p,&cmd);
		if(ret==0)
		{   
            if(cmd.ctrl_cmd!=0 || cmd.info_cmd!=0 ||cmd.set_mode!=0)
                printf("\n**********[get_command:%d]",__LINE__);
            if(cmd.ctrl_cmd!=0)
            {
                printf("ctrl_cmd=%x ",cmd.ctrl_cmd);
                if((cmd.ctrl_cmd & CMD_PLAY) || (cmd.ctrl_cmd & CMD_PLAY_START))
                    printf("filename=%s ",cmd.filename);
                else
                    printf("param=%d %d ", cmd.param,cmd.param1);
            }
            if(cmd.info_cmd!=0)
                printf("info_cmd=%x ",cmd.info_cmd);
            if(cmd.set_mode!=0)
                printf("set_mode=%x ",cmd.set_mode);            
            printf("\n");

            ret = parse_command(p,&cmd);     
    		if(ret<0)
            {
                if(p->background && ret == INVALID_CMD)                                                        
                    log_print("invalid command under current player state\n");    
                else
                {
                    log_print("[controler_run:%d]parse command return %d,break from loop!\n",__LINE__,ret);
                    break;
                }
    		}            
        }
		else if(ret == 0x5a5a)
		{
            log_print("[controler_run:%d]0x5a5a,break from loop!\n",__LINE__);
            break;     
		}

        ret = player_list_allpid(&player_id);    
        //log_print("[controler_run:%d]get_player_state=%d i=%d pid=%d ret=%d num=%d\n",__LINE__,pstatus,i,pid,ret,player_id.num);

        //check all player_thread player status    
        int stop_flag = 0,error_flag = 0,end_flag = 0;
        for(i = 0; i < player_id.num; i++)
        {
            pid = player_id.pid[i]; 
            pstatus = player_get_state(pid);  

            if(pstatus==PLAYER_STOPED)  
                stop_flag ++;
            else if(pstatus==PLAYER_ERROR)   
                error_flag ++;
            else if(pstatus==PLAYER_PLAYEND && (!p->thread_ctrl_para[i]->play_ctrl_para.loop_mode)) 
                end_flag ++;
            
            if((pstatus==PLAYER_STOPED)||(pstatus==PLAYER_ERROR)||
               (!p->thread_ctrl_para[i]->play_ctrl_para.loop_mode && pstatus==PLAYER_PLAYEND))
            {                
                wait_thread_exit(i);
                destruct_thread_para(p->thread_ctrl_para[i]);    
                player_release(i);
                player_release_pid(i);   
            }            
        }        
        if(p->background)
            continue;
        else if(((stop_flag + error_flag + end_flag) == player_id.num) &&(player_id.num > 0))
        {
            log_print("[controler_run:%d]stop=%d error=%d end=%d num=%d\n",__LINE__,stop_flag,error_flag,end_flag,player_id.num);
            break;  /*exit when is  not backmode*/                      
        }
    }while(1);
    
	if(ctrl->release)
		ctrl->release(p);    
}


