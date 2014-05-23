

#ifndef PALYER_CTROL_H
#define PALYER_CTROL_H

#include "player_type.h"
#include "controler.h"
struct player_controler;
typedef struct control_para
{

	int     argc;
	char    *argv;	
    int	  	background; //running as daemon
	char 	control_mode[16];
	char    *socket_path;
	char	*socket_rsp_path;
    play_control_t g_play_ctrl_para;
	thread_ctrl_para_t *thread_ctrl_para[MAX_PLAYER_THREADS];	
	struct player_controler *controler;
	void* externlibfd[10];
}global_ctrl_para_t;

#define  MALLOC(s)		malloc(s)
#define  FREE(d)		free(d)
#define  MEMCPY(d,s,l)	memcpy(d,s,l)
#define  MEMSET(d,v,l)		memset(d,v,l)
#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define LOG_FILE		"/tmp/amplayer2.log"

int construct_thread_para(thread_ctrl_para_t *t, play_control_t *g);
int destruct_thread_para(thread_ctrl_para_t *t);

#endif
