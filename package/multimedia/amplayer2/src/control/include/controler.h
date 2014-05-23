#ifndef  CONTROL_H_SSSSSSS
#define	CONTROL_H_SSSSSSS

#include <player.h>
#include <message.h>
#include <player_ctrl.h>
struct control_para;


typedef struct player_controler
{
		char name[16];
		int front_mode;
		int (*init)(struct control_para *); 
		int (*get_command)(player_cmd_t *);
		int (*update_state)(int pid,player_info_t*);
        int (*ret_info)(int pid, int cid, int type, void *data);
		int (*release)(struct control_para *); 
}player_controler_t;

#define IS_MODE(s,mode) (strlen(s)==strlen(mode) && !memcmp(s,mode,strlen(s)))
void basic_controlers_init(void);
void controler_run(struct control_para *p);
int start_controler(struct control_para *player_para);
int register_controler(player_controler_t *controler);

#endif

