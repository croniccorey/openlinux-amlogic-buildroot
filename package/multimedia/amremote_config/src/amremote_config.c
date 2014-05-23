
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h> 
#include <linux/input.h>
#include  <asm/ioctl.h>
#include "remote_config.h"
//remote config  ioctl  cmd



extern remote_config_t  *remote_device_config(unsigned int factory_code,int dump_enable);


int  usage(void)
{
	printf("[usage]  \n"\
			"\t amremote_config  [factory_code] \n"\
			"[examble]\n"\
			"\t amremote_config 0xff000001\n");
}

int main(int  argc,char *argv[])
{
	remote_config_t  *config;
	unsigned int  factory_code;

	if(argc != 2 ) return usage(); 
	factory_code=strtoul(argv[1],NULL,0);
	if( (config=remote_device_config(factory_code,1))==NULL)
	{
		printf("can't get config parameter\r\n");
		return -1;
	}
	return 0;
}

