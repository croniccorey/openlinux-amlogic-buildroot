#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "msgmngr.h"
#include "msgmngr_internal.h"
#include "msgqueue.h"
#include "comm_socket_tcp.h"
#include "taskmngr.h"
#include "mp_log.h"

#define RECV_TIMEOUT_MAX 200
static pGetMsgCallback pfunc = NULL;
static void* pObj = NULL;
static int cli_fd = -1;//about client connected server

static int  tid_post = -1;
static int  tid_recv = -1;

static int client_id = -1;

static int isConnect = -1;

//================================================


static unsigned int req_seq_no_base = 11; 
static pthread_mutex_t req_seq_mutex = PTHREAD_MUTEX_INITIALIZER;

#define REQ_SEQ_LOCK()   pthread_mutex_lock(&req_seq_mutex)
#define REQ_SEQ_UNLOCK() pthread_mutex_unlock(&req_seq_mutex)

#define _INSTID_POOL_MAX 16
typedef enum{
    INSTID_NOT_USED = 0,
    INSTID_USED_IN,
    INSTID_BINDED_OK,
}MP_INSTID_STATE;


typedef struct _MP_instid_map{
    int mpinstID;
    int pid;   
    MP_INSTID_STATE state;
}MP_instid_map_node;


static MP_instid_map_node _instid_pool[_INSTID_POOL_MAX];

static int mplayerinstid_counts = 0;

int open_player_instance(int mplayerinstid);

int clearall_instid_pool()
{
    REQ_SEQ_LOCK();

    memset((void*)&_instid_pool,0,sizeof(MP_instid_map_node)*_INSTID_POOL_MAX);
    req_seq_no_base = 11;
    REQ_SEQ_UNLOCK();    

    return 0;
}

int open_player_instance(int mplayerinstid)
{
    int i = -1; ;
    if(mplayerinstid <11&&mplayerinstid_counts>=_INSTID_POOL_MAX)
        return -1;
   
    for(i =0;i<_INSTID_POOL_MAX;i++)
    {
        if(_instid_pool[i].state == INSTID_NOT_USED)
        {
            _instid_pool[i].mpinstID = mplayerinstid;
             _instid_pool[i].state = INSTID_USED_IN;
             
            mplayerinstid_counts++;
            log_info("current media player instance num:%d\n",mplayerinstid_counts);
            break;            
        }
    }
    
    return mplayerinstid;
}

int cli_bind_inst_with_amplayer(int mplayerinstid,int pid)
{
    int i = -1;
    int isFound = -1;
    if(mplayerinstid < 11)
        return -1;
    REQ_SEQ_LOCK();  
    for(i=0;i<_INSTID_POOL_MAX;i++)
    {
       
        if(_instid_pool[i].state == INSTID_USED_IN&&_instid_pool[i].mpinstID == mplayerinstid)
        {
            _instid_pool[i].pid = pid;
            _instid_pool[i].state = INSTID_BINDED_OK;
            log_info("bind media player instance id[%d] with amplayer task id[%d]\n",mplayerinstid,pid);
            isFound = 1;
            break;            
        }
    }
    REQ_SEQ_UNLOCK();  
    return isFound;   
    
}

int cli_get_amplayer_task_id(int mplayerinstid)
{
    int pid = -1;
    int i = -1;
    REQ_SEQ_LOCK();      
    for(i=0;i<_INSTID_POOL_MAX;i++)
    {
        if(_instid_pool[i].state == INSTID_BINDED_OK&&_instid_pool[i].mpinstID == mplayerinstid)
        {
            pid = _instid_pool[i].pid;         
            log_info("get amplayer task id[%d] with instance id:%d\n",pid,mplayerinstid);  
            break;            
        }
    }
    REQ_SEQ_UNLOCK();  
    return pid;
}

int cli_get_mplayer_inst_id(int pid)
{
    int i =-1;
    int mplayerinstid = -1;
    REQ_SEQ_LOCK();  
    for(i =0;i<_INSTID_POOL_MAX;i++)
    {
        if(_instid_pool[i].state == INSTID_BINDED_OK&&_instid_pool[i].pid == pid)
        {
            mplayerinstid = _instid_pool[i].mpinstID;         
            log_info("get mplayer instance id[%d] with amplayer task id:%d\n",mplayerinstid,pid);  
            break;            
        }
    }   
    REQ_SEQ_UNLOCK();  
    return mplayerinstid;
}

int cli_close_player_instance(int mplayerinstid)
{
    int pid = -1;
    int isFound = -1;
    int i =-1;
    REQ_SEQ_LOCK();  
    for(i =0;i<_INSTID_POOL_MAX;i++)
    {
        if(_instid_pool[i].mpinstID == mplayerinstid)
        {
            memset((void*)&_instid_pool[i],0,sizeof(MP_instid_map_node));    
            log_info("close player instance,id[%d]\n",mplayerinstid);  
            isFound = 1;
            break;            
        }
    }
    
    REQ_SEQ_UNLOCK();  
    return isFound;    
}
    
//about client context
typedef struct{
    char svr_ipc_name[IPC_NAME_MAX];
    int queue_size;
}CliThreadArg;


static pthread_mutex_t socket_connect_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SOCKET_RETRY_LOCK()   pthread_mutex_lock(&socket_connect_mutex)
#define SOCKET_RETRY_UNLOCK() pthread_mutex_unlock(&socket_connect_mutex)


static void* client_recv_msg_thread_tcp(void* arg);
static void* client_post_msg_thread(void* arg);

//===============common====================//
int init_client_msg_mngr(const char*tcp_ipc_name,int postQueueSize,void* pt2Obj,pGetMsgCallback pf)
{

    CliThreadArg arg;

    if(pf)
        pfunc = pf;//init callback function;

    if(pt2Obj)
        pObj = pt2Obj;

    memset((void*)&arg,0,sizeof(CliThreadArg));
    
    memset((void*)&_instid_pool,0,sizeof(MP_instid_map_node)*_INSTID_POOL_MAX);
    
    if(postQueueSize > 0)
        arg.queue_size = postQueueSize;
    else
        arg.queue_size = DEFAULT_QUEUE_SIZE;

    if(NULL != tcp_ipc_name&&strlen(tcp_ipc_name)<IPC_NAME_MAX)
        strncpy(arg.svr_ipc_name,tcp_ipc_name,IPC_NAME_MAX);
    else
        strncpy(arg.svr_ipc_name,COMM_TCP_HANDLE,IPC_NAME_MAX);

    MP_taskBlockSignalPE();
    
    MP_taskSetCancel();
    
    tid_post = MP_taskCreate("client_post_msg_thread",1, 409600,client_post_msg_thread,&arg);

    if(tid_post == -1)
    {        
        log_err("Failed to create client post thread !\n");
        return -1;
    }
    tid_recv = MP_taskCreate("client_recv_msg_thread_tcp",1, 409600,client_recv_msg_thread_tcp,&arg);

    if(tid_recv == -1)
    {
        MP_taskDelete(tid_post);
        log_err("Failed to create client receive thread !\n");
        return -1;
    }
    return 0;
}



int uninit_client_msg_mngr()
{
    if(tid_recv != -1)
        MP_taskDelete(tid_recv);
    if(tid_post != -1)
        MP_taskDelete(tid_post);
    if(cli_fd > 0)
        disconnect_stream(cli_fd);
    
    //clearall_message_queue();

    cli_fd =-1;
    tid_recv = -1;
    tid_post = -1;

    pfunc = NULL;
    pObj = NULL;
    log_info("[%s] Succeed to uninit client message manager\n",__FUNCTION__);

    return 0;

}

int register_get_msg_func(void* pt2Obj,pGetMsgCallback func)
{
    if(func)
    {
        pfunc = func;
        pObj= pt2Obj;
        log_info("succeed to register function\n");
        return 0;
        
    }

    pfunc = NULL;
    pObj = NULL;
        
    log_info("succeed to un-register function\n");
    return -1;
}
int test_client_connect_status()
{//remove mutex lock. be care!
    return isConnect ;   
}
int cli_post_message(const void* buf,int msg_len)
{
    if(NULL != buf && msg_len >0&&msg_len <= 1500)
    {
        struct msg_item_t *item = NULL;
        item = mw_alloc_msg_item(msg_len);
        if(NULL!= item&&item != -1)
        {
            memcpy(item->msg_ptr,buf,msg_len);
            mw_post_to_message_queue(item);
        }
        else
            log_wrn("Message queue is full,drop this message!\n");

    }
    return 0;
}


int cli_send_message(const void *buf, unsigned int msg_len,void *reply, unsigned int *rsize,int timeout)
{
    int ret = -1;   
    unsigned char dat[MESSAGE_SIZE_MAX];
    if(NULL == buf&& NULL == reply && -1 != cli_fd&& msg_len > 1500)
    {//sanity check
        log_err("Never input null input data or invalid output buf\n");
        return -1;
    }
    memcpy((void*)&dat,buf,MESSAGE_SIZE_MAX);
    
    memcpy((void*)&dat,(void*)&client_id,sizeof(int));//with agent_id
    
    MP_taskSuspend(tid_recv);   
    ret = send_msg(cli_fd,dat,msg_len,reply,rsize,timeout);
    MP_taskResume(tid_recv);
    return ret;    
}

//===============client about====================//

#define CLI_RETRY_COUNT 3
#define CLI_RETRY_NOTIFY 3
static void* client_post_msg_thread(void *arg)
{
    int flag = 1;
    int timeo = -1;
    int ret = -1;
    char svr_ipc[IPC_NAME_MAX];
    int poolsize = ((CliThreadArg*)arg)->queue_size;   
    int wrn_notify_retry = 0;

    strncpy(svr_ipc,((CliThreadArg*)arg)->svr_ipc_name,IPC_NAME_MAX);

    ret = mw_initialize_msg_pool(poolsize);//only has five slot for cache msg.
    if(ret != 0)
    {
        log_err("failed to init message queue,terminate current thread\n");
        return NULL;
    }    

    while(flag)
    {

        SOCKET_RETRY_LOCK();
        if(cli_fd <= 0)
        {
            cli_fd = connect_stream(svr_ipc,CLI_RETRY_COUNT,&client_id);
            if(cli_fd <=0)
            {
                SOCKET_RETRY_UNLOCK();
                usleep(1000000);//1000ms
                if(wrn_notify_retry <CLI_RETRY_NOTIFY)
                {
                     log_wrn("failed to connect,go to connect flow\n");
                     wrn_notify_retry++;
                }
                continue;
            }
            isConnect = 1;
            log_info("[%s]client has connected with server,client ID:%d\n",__FUNCTION__,client_id);
        }
        SOCKET_RETRY_UNLOCK();

        while(flag)
        {            
            
            struct msg_item_t *item = NULL;
            //wait until message received
            item = mw_dequeue_message_queue(timeo);
            if(item)
            {                
                memcpy(item->msg_ptr,(void*)&client_id,sizeof(int));//with agent_id
                //MP_taskTestCancel();
                ret = post_msg(cli_fd,item->msg_ptr,item->msg_len);
                //MP_taskTestCancel();

                mw_free_msg_item(item);

                if(ret != 0)
                {
                    SOCKET_RETRY_LOCK();
                    disconnect_stream(cli_fd);
                    cli_fd = -1;
                    isConnect = -1;
                    SOCKET_RETRY_UNLOCK();
                    mw_clearall_message_queue();
                    clearall_instid_pool();
                    log_wrn("[%s]failed to post data to server,reconnect\n",__FUNCTION__);
                    break;
                }
            }           
        }
    }

    mw_free_msg_pool();
    return NULL;
}

static void* client_recv_msg_thread_tcp(void* arg)
{
    int flag = 1;
    UINT8 tmp[COMMON_MSG_SIZE];
    unsigned int len = -1;
    int ret = -1;
    char svr_ipc[IPC_NAME_MAX];
    int wrn_notify_retry = 0;
    strncpy(svr_ipc,((CliThreadArg*)arg)->svr_ipc_name,IPC_NAME_MAX);
    while(flag)
    {

        SOCKET_RETRY_LOCK();
        if(cli_fd <= 0)
        {
            cli_fd = connect_stream(svr_ipc,CLI_RETRY_COUNT,&client_id);

            if(cli_fd <=0)
            {
                SOCKET_RETRY_UNLOCK();
                usleep(1000000);//1000ms
                if(wrn_notify_retry <CLI_RETRY_NOTIFY)
                {
                     log_wrn("failed to connect,go to connect flow\n");
                     wrn_notify_retry++;
                }              
                continue;
            }
            isConnect = 1;
            log_info("[%s]client has connected with server,client ID:%d\n",__FUNCTION__,getpid());
        }
        SOCKET_RETRY_UNLOCK();
        while(flag)
        {
            memset(tmp,0,COMMON_MSG_SIZE);
            //MP_taskTestCancel();            
            ret = recv_msg(cli_fd,&tmp,&len,RECV_TIMEOUT_MAX);
            //MP_taskTestCancel();

            if(ret == -1)
            {
                SOCKET_RETRY_LOCK();
                disconnect_stream(cli_fd);
                cli_fd = -1;
                isConnect = -1;
                SOCKET_RETRY_UNLOCK();
                mw_clearall_message_queue();
                clearall_instid_pool();
                log_wrn("failed to receive data to server,reconnect\n");
                break;
            }

            
            if(ret == 0 &&len >0 &&pfunc != NULL)
            {
                //ugly codes,just for amplayer 
                log_con("============start to ugly trick codes=============\n");
                int inst_id = -1;
                int pid =-1;
                int rv =-1;
                memcpy((void*)&inst_id,tmp+4,sizeof(int));
                log_info("receive instance id:%d\n",inst_id);
                if(inst_id > 0)
                {
                    memcpy((void*)&pid,tmp+16,sizeof(int));  
                
                    rv = cli_bind_inst_with_amplayer(inst_id,pid);
                    if(rv < 0 )
                    {
                        log_info("two id binded failed\n");
                        continue;
                    }
                   
                    //memcpy(tmp+16,(void*)&inst_id,sizeof(int));     //drop a bind message.              
                    continue;              
                                    
                }
                else
                {
                    memcpy((void*)&pid,tmp+16,sizeof(int));
                    log_info("current pid is :%d\n",pid);
                    inst_id = cli_get_mplayer_inst_id(pid);
                    if(inst_id >0)
                    {
                        memcpy(tmp+16,(void*)&inst_id,sizeof(int));
                    }
                    else
                    {
                        
                        log_trc("maybe volume's abouts,can't find valid instance id,amplayer pid:%d\n",pid); 
                        
                    }

                }
                pfunc(pObj,tmp,len);
            }
            else
            {
                MP_taskDelay(10000);//10ms
            }

        }

    }
    return NULL;
}

int cli_get_request_seq_no()
{
    int ret =-1;
    int inst =0;
    REQ_SEQ_LOCK();

    inst= req_seq_no_base;

    ret = open_player_instance(req_seq_no_base);
    if(ret <0)
        ret = -1;
    if(req_seq_no_base < 100)
        req_seq_no_base ++;
    else
        req_seq_no_base = 11;
   
    
    REQ_SEQ_UNLOCK();
        
    return ret;
    
}

