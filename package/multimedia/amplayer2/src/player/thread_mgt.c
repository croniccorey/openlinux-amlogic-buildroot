/********************************************
 * name	: player_control.c
 * function: thread manage
 * date		: 2010.2.22
 ********************************************/
#include <stdio.h>
#include <sys/time.h> 
#include <time.h> 
#include <pthread.h>
#include "player_priv.h"
#include "player.h"
#include "player_error.h"
#include "thread_mgt.h"


int  player_thread_wait(play_para_t *player,int microseconds)
{
    struct timespec pthread_ts; 
    struct timeval now;
	player_thread_mgt_t *mgt=&player->thread_mgt;
	int ret;
	
	gettimeofday (&now, NULL);   
    pthread_ts.tv_sec = now.tv_sec + microseconds/1000000;
    pthread_ts.tv_nsec =  microseconds*1000;
    pthread_mutex_lock(&mgt->pthread_mutex); 
    ret=pthread_cond_timedwait(&mgt->pthread_cond, &mgt->pthread_mutex, &pthread_ts);
    pthread_mutex_unlock(&mgt->pthread_mutex);
	return ret;
}
int wakeup_player_thread(play_para_t *player)
{
	player_thread_mgt_t *mgt=&player->thread_mgt;
	int ret;
	
	pthread_mutex_lock(&mgt->pthread_mutex); 
	ret=pthread_cond_signal(&mgt->pthread_cond);
	pthread_mutex_unlock(&mgt->pthread_mutex);

	return ret;
}

player_status get_player_state(play_para_t *player)
 {
    player_status status = PLAYER_UNKNOWN;
 	player_thread_mgt_t *mgt=&player->thread_mgt;
    if(mgt)
 	    status = mgt->player_state;
    return status;
 }

 int player_thread_create(play_para_t *player)
{
	int ret =0;
	pthread_t       tid;  	
	pthread_attr_t pthread_attr;
	player_thread_mgt_t *mgt=&player->thread_mgt;
	
	pthread_attr_init(&pthread_attr);
	pthread_attr_setstacksize(&pthread_attr,409600);
	pthread_mutex_init(&mgt->pthread_mutex,NULL);	
	pthread_cond_init(&mgt->pthread_cond,NULL);
	printf("***player_para=%p,start_param=%p\n",player,player->start_param);
	ret = pthread_create(&tid, &pthread_attr, (void*)&player_thread, (void*)player);	
	if(ret != 0)
	{
		log_print("creat player thread failed !\n");
		return ret;
	}
	
	mgt->pthread_id=tid; 
	return PLAYER_SUCCESS;
	
}
void player_thread_wait_exit(play_para_t *player)
 {  
 	player_thread_mgt_t *mgt=&player->thread_mgt;
 	pthread_join(mgt->pthread_id,NULL); 	 
 }

#ifdef PLAYER_DEBUG 
void debug_set_player_state(play_para_t *player,player_status status,const char *fn,int line)
  {
	  player_thread_mgt_t *mgt=&player->thread_mgt;
	  log_print("\n************************Changed player state to %d,At %s:%d\n",status,fn,line);
	  mgt->player_state=status;
  }
#else
void set_player_state(play_para_t *player,player_status status)
  {
	  player_thread_mgt_t *mgt=&player->thread_mgt;
	  mgt->player_state=status;
  }
#endif
	
