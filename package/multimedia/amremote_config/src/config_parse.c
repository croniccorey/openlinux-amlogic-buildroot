#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string.h>
#include <time.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h> 
#include <linux/input.h>
#include  <asm/ioctl.h>
#include <errno.h>
#include "remote_config.h"

#define  CONFIG_FILE_NAME   "/etc/remote.conf"
#define  STR_FACTORY_CODE "factory_code"

//these string must in this order and sync with struct remote_config_t
  static char*  config_item[16]={
	"work_mode",
	"repeat_enable",
	"factory_code",
	"release_delay",
	"debug_enable",
//sw
	"bit_count",
	"tw_leader_act",
	"tw_bit0",
	"tw_bit1",
	"tw_repeat_leader",
//reg
	"reg_base_gen",
	"reg_control",
	"reg_leader_act",
	"reg_leader_idle",
	"reg_repeat_leader",
	"reg_bit0_time"};

static int   remote_ioc_table[16]={
	REMOTE_IOC_SET_MODE,
	REMOTE_IOC_SET_REPEAT_ENABLE,
	REMOTE_IOC_SET_CUSTOMCODE,
	REMOTE_IOC_SET_RELEASE_DELAY,
	REMOTE_IOC_SET_DEBUG_ENABLE,
//sw
	REMOTE_IOC_SET_BIT_COUNT,
	REMOTE_IOC_SET_TW_LEADER_ACT,
	REMOTE_IOC_SET_TW_BIT0_TIME,
	REMOTE_IOC_SET_TW_BIT1_TIME,
	REMOTE_IOC_SET_TW_REPEATE_LEADER,
//reg
	REMOTE_IOC_SET_REG_BASE_GEN,
	REMOTE_IOC_SET_REG_CONTROL	,
	REMOTE_IOC_SET_REG_LEADER_ACT,
	REMOTE_IOC_SET_REG_LEADER_IDLE,
	REMOTE_IOC_SET_REG_REPEAT_LEADER,
	REMOTE_IOC_SET_REG_BIT0_TIME};

	
static void
str_trim( char **s )
{
     int i;
     int len=strlen(*s) ;

     for (i = len-1; i >= 0; i--)
          if ((*s)[i] <= ' ')
               (*s)[i] = 0;
          else
               break;

     while (**s)
          if (**s <= ' ')
               (*s)++;
          else
               return;
}
static int  remote_config_set(char *name,char *value,remote_config_t  *config)
{
	int i;
	unsigned  int *config_para=(unsigned int*)config;
	for(i=0;i<ARRAY_SIZE(config_item);i++)
	{
		if(strcmp(config_item[i],name)==0)
		{
			//printf("curpara:%s\r\n",name);
			config_para[i]=strtoul(value,NULL,0);
			break;
		}
	}
	return 0;
}
void  set_config( remote_config_t  *config,int config_fd,int dump_enable)
{
	int i;
	unsigned int *para=(unsigned int*)config;
	
	for(i=0;i<ARRAY_SIZE(config_item);i++)
	{
		if(para[i]!=0x77777777)
		{	
			switch(i)
			{
				case 2:
				case 6:
				case 7:
				case 8:
				case 9:	
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				if(dump_enable)	
				printf("%s=0x%x\r\n",config_item[i],para[i]);	
				break;
				default:
				if(dump_enable)	
				printf("%s=%d\r\n",config_item[i],para[i]);
				break;
			}
			switch(i)
			{
				case 2:
				config->factory_code>>=16;	
				ioctl	(config_fd,REMOTE_IOC_SET_CUSTOMCODE,&config->factory_code );
				break;
				default:
				ioctl	(config_fd,remote_ioc_table[i],&para[i]);	
				break;	
			}
		}
	}
}
remote_config_t  *remote_device_config(unsigned int factory_code,int dump_enable)
{
	FILE *fd;
	char  line[400];
	char  factory_match=0;
	char  skip_enable=1;
	int ret ;
	static  remote_config_t  config;
	

	memset((void*)&config,0x7777,sizeof(remote_config_t ));
	config.factory_code=factory_code;
	
	fd=fopen(CONFIG_FILE_NAME,"r");
	if(!fd)
	{
		printf("can't open config file:%s\r\n",strerror(errno));
		return NULL;
	}
	 while (fgets( line, 400, fd)) {
       	char *name = line;
          	char *comment = strchr((const char*) line,'#');
          	char *value;

	  	if (comment) {
             *comment = 0;
          	}

	       value = strchr( line, '=' );

          	if (value) {
               *value++ = 0;
               str_trim( &value );
          	}
		str_trim( &name );
		
          	if (!*name  ||  *name == '#')
             continue;
		if(strcmp((const char*)name,STR_FACTORY_CODE)==0 && factory_match==1)
		{
			break ; //all config has been set 
		}
		if(skip_enable)
		{
			if(strcmp(name,STR_FACTORY_CODE)!=0)
			continue;	
		}
		if(strcmp((const char*)name,STR_FACTORY_CODE)==0 && factory_match==0)
		{
			factory_code=strtoul(value,NULL,0);
			if(factory_code != config.factory_code)
			{
				continue ;
			}
			factory_match=1;
			skip_enable=0;
			continue ;
		}
          	ret = remote_config_set( name, value,&config );
          	if (ret) {
             	printf("config file has not supported parameter:%s=%s\r\n",name,value);
          	}
     }
     	
     fclose( fd );
     if(factory_match)
     {
     		int config_fd;
     		config_fd=open("/dev/amremote",O_RDWR);
		if(config_fd<0)
		{	
			printf("can't open remote config file\r\n");
			return NULL;
		}
		set_config(&config,config_fd,dump_enable);
		close(config_fd);
     		return &config;
     }
	return NULL;
}
