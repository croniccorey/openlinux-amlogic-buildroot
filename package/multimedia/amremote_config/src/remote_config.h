#ifndef  _REMOTE_CONFIG_H
#define  _REMOTE_CONFIG_H

#define   REMOTE_IOC_SET_REPEAT_ENABLE			_IOW('I',8,sizeof(short))
#define	REMOTE_IOC_SET_DEBUG_ENABLE			_IOW('I',9,sizeof(short)) 
#define	REMOTE_IOC_SET_MODE					_IOW('I',10,sizeof(short)) 

#define   REMOTE_IOC_SET_RELEASE_DELAY		_IOW('I',99,sizeof(short))
#define   REMOTE_IOC_SET_CUSTOMCODE   			_IOW('I',100,sizeof(short))
//reg
#define   REMOTE_IOC_SET_REG_BASE_GEN			_IOW('I',101,sizeof(short))
#define   REMOTE_IOC_SET_REG_CONTROL			_IOW('I',102,sizeof(short))
#define   REMOTE_IOC_SET_REG_LEADER_ACT 		_IOW('I',103,sizeof(short))
#define   REMOTE_IOC_SET_REG_LEADER_IDLE 		_IOW('I',104,sizeof(short))
#define   REMOTE_IOC_SET_REG_REPEAT_LEADER 	_IOW('I',105,sizeof(short))
#define   REMOTE_IOC_SET_REG_BIT0_TIME		 	_IOW('I',106,sizeof(short))

//sw
#define   REMOTE_IOC_SET_BIT_COUNT			 	_IOW('I',107,sizeof(short))
#define   REMOTE_IOC_SET_TW_LEADER_ACT		_IOW('I',108,sizeof(short))
#define   REMOTE_IOC_SET_TW_BIT0_TIME			_IOW('I',109,sizeof(short))
#define   REMOTE_IOC_SET_TW_BIT1_TIME			_IOW('I',110,sizeof(short))
#define   REMOTE_IOC_SET_TW_REPEATE_LEADER	_IOW('I',111,sizeof(short))

#define   REMOTE_IOC_GET_TW_LEADER_ACT		_IOR('I',112,sizeof(short))
#define   REMOTE_IOC_GET_TW_BIT0_TIME			_IOR('I',113,sizeof(short))
#define   REMOTE_IOC_GET_TW_BIT1_TIME			_IOR('I',114,sizeof(short))
#define   REMOTE_IOC_GET_TW_REPEATE_LEADER	_IOR('I',115,sizeof(short))


#define   REMOTE_IOC_GET_REG_BASE_GEN			_IOR('I',121,sizeof(short))
#define   REMOTE_IOC_GET_REG_CONTROL			_IOR('I',122,sizeof(short))
#define   REMOTE_IOC_GET_REG_LEADER_ACT 		_IOR('I',123,sizeof(short))
#define   REMOTE_IOC_GET_REG_LEADER_IDLE 		_IOR('I',124,sizeof(short))
#define   REMOTE_IOC_GET_REG_REPEAT_LEADER 	_IOR('I',125,sizeof(short))
#define   REMOTE_IOC_GET_REG_BIT0_TIME		 	_IOR('I',126,sizeof(short))
#define   REMOTE_IOC_GET_REG_FRAME_DATA		_IOR('I',127,sizeof(short))
#define   REMOTE_IOC_GET_REG_FRAME_STATUS	_IOR('I',128,sizeof(short))


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef   struct{
	unsigned int work_mode ;
	unsigned int repeat_enable;
	unsigned int factory_code;
	unsigned int release_delay;
	unsigned int debug_enable;
//sw
	unsigned int 	bit_count;
	unsigned int 	tw_leader_act;
	unsigned int 	tw_bit0;
	unsigned int   tw_bit1;
	unsigned int 	tw_repeat_leader;
//reg
	unsigned int  reg_base_gen;
	unsigned int  reg_control;
	unsigned int  reg_leader_act;
	unsigned int  reg_leader_idle;
	unsigned int  reg_repeat_leader;
	unsigned int  reg_bit0_time;
	
	
}remote_config_t;
#endif
