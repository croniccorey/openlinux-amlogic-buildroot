#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <player.h>
#include <string.h>

#include <pthread.h>
#include "player_priv.h"

int ffmpeg_lock(void **pmutex, enum AVLockOp op)
{
	int r;
	pthread_mutex_t *mutex=*pmutex;
	switch(op)
		{
		case AV_LOCK_CREATE:  ///< Create a mutex
			  mutex=MALLOC(sizeof(pthread_mutex_t));
			  if(mutex==NULL)
			  		return -1;
			  r=pthread_mutex_init(mutex,NULL);
			  if(r!=0)
				{
				FREE(mutex); 
				mutex=NULL;
				}
			  *pmutex=mutex;
			  break;
		case AV_LOCK_OBTAIN:  ///< Lock the mutex
			  r=pthread_mutex_lock(mutex); 
			  break;
		case AV_LOCK_RELEASE: ///< Unlock the mutex
			  r=pthread_mutex_unlock(mutex); 
			  break;
		case AV_LOCK_DESTROY: ///< Free mutex resources
			  if(mutex)
			  	FREE(mutex); 
			  *pmutex=NULL;
			  break;
		}
	return r;
}


int ffmpeg_init(void)
{
	av_register_all();
	av_lockmgr_register(ffmpeg_lock);

	return 0;
}

