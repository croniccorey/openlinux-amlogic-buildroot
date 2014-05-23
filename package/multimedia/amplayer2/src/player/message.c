#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h> 
#include <time.h> 
#include <player.h>

#include "player_priv.h"
#include "player_update.h"
#include "thread_mgt.h"

#include <message.h>




int message_pool_init(play_para_t *para)
{	
	message_pool_t *pool=&para->message_pool;
	pool->message_num=0;
	pool->message_in_index=0;
	pool->message_out_index=0;
	MEMSET(pool->message_list,0,sizeof(pool->message_list));
	pthread_mutex_init(&pool->msg_mutex,NULL);
	pool->update_statue_callback=NULL;
	pool->update_interval=1000;
	return 0;
}
player_cmd_t * message_alloc(void)
{
	player_cmd_t *cmd;
	cmd=MALLOC(sizeof(*cmd));
	if(cmd)
	{
	    MEMSET(cmd, 0, sizeof(player_cmd_t));
	}
	return cmd;
}

int message_free(player_cmd_t * cmd)
{
	if(cmd)
		FREE(cmd);
	return 0;
}


int send_message(play_para_t *para,player_cmd_t *cmd)
{
	int ret=-1;
	message_pool_t *pool=&para->message_pool;
	//printf("[send_message:%d]num=%d in_idx=%d out_idx=%d\n",__LINE__,pool->message_num,pool->message_in_index,pool->message_out_index);
	pthread_mutex_lock(&pool->msg_mutex);
	 if(pool->message_num<MESSAGE_MAX){        
	 	pool->message_list[pool->message_in_index]=cmd;
		pool->message_in_index=(pool->message_in_index+1)%MESSAGE_MAX;
		pool->message_num++;
		wakeup_player_thread(para);
		ret=0;
	 	}
	 else
	 	{
	 	/*message_num is full*/
		player_cmd_t *oldestcmd;
		oldestcmd=pool->message_list[pool->message_in_index];
		FREE(oldestcmd);
		pool->message_out_index=(pool->message_out_index+1)%MESSAGE_MAX;/*del the oldest command*/
		pool->message_list[pool->message_in_index]=cmd;
		pool->message_in_index=(pool->message_in_index+1)%MESSAGE_MAX;
		wakeup_player_thread(para);
		ret=0;
	 	}
	 pthread_mutex_unlock(&pool->msg_mutex);
     printf("[send_message:%d]num=%d in_idx=%d out_idx=%d cmd=%x mode=%d\n",__LINE__,pool->message_num,pool->message_in_index,pool->message_out_index,cmd->ctrl_cmd, cmd->set_mode);
	return ret;
	
}

int send_message_by_pid(int pid,player_cmd_t *cmd)
 {	
 	int ret;
 	play_para_t *player_para;
	player_para=player_open_pid_data(pid);
	if(player_para==NULL)
		return PLAYER_NOT_VALID_PID;
	ret=send_message(player_para,cmd);
	player_close_pid_data(pid);
	return ret;
 }

player_cmd_t * get_message(play_para_t *para)
{
	player_cmd_t *cmd=NULL;
	message_pool_t *pool=&para->message_pool;
    if(pool == NULL)
    {
        log_error("[get_message]pool is null!\n");
        return NULL;
    }    
	pthread_mutex_lock(&pool->msg_mutex);
    //printf("[get_message]pool=%p msg_num=%d\n",pool,pool->message_num);
	 if(pool->message_num>0){
	 	
	 	cmd=pool->message_list[pool->message_out_index];
		pool->message_out_index=(pool->message_out_index+1)%MESSAGE_MAX;
		pool->message_num--;
        //printf("[get_message:%d]num=%d in_idx=%d out_idx=%d cmd=%x\n",__LINE__,pool->message_num,pool->message_in_index,pool->message_out_index,cmd->cmd);
	 	}
	pthread_mutex_unlock(&pool->msg_mutex);    
	return cmd;
}
void clear_all_message(play_para_t *para)
{
	player_cmd_t *cmd;
	while((cmd=get_message(para))!=NULL)
		{
		message_free(cmd);
		}
}
int register_update_callback(play_para_t *para,update_state_fun_t up_fn,int interval_s)
{
	message_pool_t *pool=&para->message_pool;
	if(up_fn!=NULL)
		pool->update_statue_callback=up_fn;
	if(interval_s>0)
		pool->update_interval=interval_s;
	return 0;
}

int update_player_states(play_para_t *para,int force)
{
	message_pool_t *pool=&para->message_pool;
	update_state_fun_t fn;	
	fn=pool->update_statue_callback;
	if(fn!=NULL)
	{  	
        if(check_time_interrupt(&pool->callback_old_time, pool->update_interval) || force)	    			  
	  	{             
	  		player_info_t state;
			MEMCPY(&state,&para->state,sizeof(state)); 
            //if(force == 1)
            //    printf("**[update_player_states]status=%d\n",state.status);
	  		fn(para->player_id,&state);  
	  	}         
	}
    //else
    //    printf("\nfn NULL!!!!!!!!!!!!!!!!!\n");
	return 0;
}
	

