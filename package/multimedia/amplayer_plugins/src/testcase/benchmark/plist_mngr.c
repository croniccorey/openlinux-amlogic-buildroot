#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

#include "taskmngr.h"
#include "mp_log.h"
#include "mp_api.h"

#include "plist_mngr.h"

typedef enum{
    MP_INVALID =100,
    MP_FLIST_IN ,
    MP_DECODED_IN,
    MP_PLIST_IN,
}MP_itemwhereIn;


typedef struct _MP_mediaItem_{
    char *url;
    int index;
    MP_itemwhereIn where_in;
    struct list_head flist;//play list,just one node 
}MP_mediaItem;


static struct list_head gFileList;
static int gTotalItemsInFList = 0;
static int gFileListInitFlag = 0;
static pthread_mutex_t mp_filelist_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MP_FLIST_LOCK()   pthread_mutex_lock(&mp_filelist_mutex)
#define MP_FLIST_UNLOCK() pthread_mutex_unlock(&mp_filelist_mutex)

//play list

int MP_AddFileToList(const char* url)
{
    
    MP_mediaItem* item = NULL;
    if(NULL == url && strlen(url) >FILE_URL_MAX&&strlen(url) ==0)
    {
        printf("url must not null and less than 1024\n");
        return -1;
    }  
    
    MP_FLIST_LOCK();
    if(gFileListInitFlag ==0)
    {
        INIT_LIST_HEAD(&gFileList);
        gFileListInitFlag = 1;
    }
    
    if(gTotalItemsInFList > 100)
    {
        return -1;

    }
    
    item = (MP_mediaItem*)malloc(sizeof(MP_mediaItem));
    if(NULL ==item)
    {
        log_err("failed to malloc memory:%s\n",__FUNCTION__);
        return -1;
    }    
    
    item->url= strndup(url,FILE_URL_MAX);
    
    INIT_LIST_HEAD(&item->flist);
    item->where_in = MP_FLIST_IN;
    gTotalItemsInFList++;
    item->index = gTotalItemsInFList;
    
    list_add(&item->flist, &gFileList);
    
    
    
    MP_FLIST_UNLOCK();

    return 0;    
    
}

int MP_RemoveFileFromListByUrl(const char* url);
int MP_RemoveFileFromListByIndex();
int MP_DumpPlayList()
{
    MP_mediaItem *pos = NULL;
    MP_mediaItem *tmp = NULL;
    printf("*******************playlist files dump start*****************************\n");
    MP_FLIST_LOCK();
    list_for_each_entry_safe(pos, tmp, &gFileList, flist)
    {
       printf("%d'st file url:%s\n",pos->index,pos->url);
    }
    MP_FLIST_UNLOCK();
    printf("*******************playlist files dump end******************************\n");
    return 0;
    
}
int MP_ClearAllPlayList()
{
    MP_mediaItem *pos = NULL;
    MP_mediaItem *tmp = NULL;
    printf("*******************Clean All items from list start*****************************\n");
    MP_FLIST_LOCK();
    list_for_each_entry_safe(pos, tmp, &gFileList, flist)
    {
        list_del(&pos->flist);
        free(pos->url);
        free(pos);
        gTotalItemsInFList--;
    }
    MP_FLIST_UNLOCK();
    printf("*******************Clean All items from list end*****************************\n");
    return 0;
}
    
//=========================================
int MP_AddItemToList(MP_mediaItem *item)
{
    if(item!=NULL)
    {
        MP_FLIST_LOCK();
        if(gFileListInitFlag ==0)
        {
            INIT_LIST_HEAD(&gFileList);
            gFileListInitFlag = 1;
        }

        if(gTotalItemsInFList > 100)
        {
            return -1;

        }

        item->index = gTotalItemsInFList;
        list_add(&item->flist, &gFileList);

        gTotalItemsInFList++;
        
        MP_FLIST_UNLOCK();

        return 0;
        
    }
    
    return -1;
}
MP_mediaItem* MP_GetItemFromList(MP_AutoPLayMode mode)
{
    MP_mediaItem* item = NULL;
    MP_FLIST_LOCK();

    if(gTotalItemsInFList == 0)
    {
        MP_FLIST_UNLOCK();
        return NULL;    
    }
    
    switch(mode)
    {
        case MP_PLAY_ORDER:
           
            if (!list_empty(&gFileList))
            {
                item = list_entry(gFileList.prev, MP_mediaItem, flist);
                list_del(&item->flist);
                gTotalItemsInFList --;    
            }
            break;
        case MP_PLAY_LOOP:
            if (!list_empty(&gFileList))
            {
                MP_mediaItem *item_new = NULL;
                item_new = (MP_mediaItem*)malloc(sizeof(MP_mediaItem));
                if(item_new == NULL)
                {
                    log_err("failed to malloc for item,%s\n",__FUNCTION__);
                    MP_FLIST_UNLOCK();
                    return  NULL;
                }
                item = list_entry(gFileList.prev, MP_mediaItem, flist);                 
                list_del(&item->flist);
                item_new->url = strdup(item->url);
                INIT_LIST_HEAD(&item_new->flist);
                item_new->index = item->index;
                item_new->where_in = item->where_in;
                list_move(&item_new->flist,&gFileList);
                
            }
            break;
        case MP_PLAY_SHUFFLE:
            break;
        case  MP_PLAY_BOTH:
            break;
        default:
            break;
    }
    
    MP_FLIST_UNLOCK();

    return item;    
    
}





//========================================

static int gAtiveMediaid = -1; //if -1,means unactive.
static MP_AutoPLayMode g_mode = MP_PLAY_ORDER;
//static pthread_mutex_t mp_playlist_mutex = PTHREAD_MUTEX_INITIALIZER;

//#define MP_PLIST_LOCK()   pthread_mutex_lock(&mp_playlist_mutex)
//#define MP_PLIST_UNLOCK() pthread_mutex_unlock(&mp_playlist_mutex)

static int task_id =-1;

static void* MP_AutoPlayTask(void* args);

int MP_StartAutoPlayTask(MP_AutoPLayMode mode)
{
    if(task_id >0)
    {
        log_err("just run a autoplay task,drop this command\n");
        return -1;
    }
    g_mode = mode;
    MP_taskBlockSignalPE();    
    MP_taskSetCancel();   
   
    task_id = MP_taskCreate("MP_AutoPlayTask",1, 409600,MP_AutoPlayTask,NULL);
    if(task_id == -1)
    {        
        log_err("Failed to create autoplay task thread !\n");
        return -1;
    }
    return 0;    
}
int MP_PauseAutoPlayTask()
{
    if(task_id >0 )
    {
        return 0;
    }
    return -1;
}


int MP_ResumeAutoPlayTask()
{

    if(task_id >0 )
    {
        return 0;
    }
    return -1;
}
int MP_SetAutoPlayMode(MP_AutoPLayMode mode)
{
    g_mode= mode;
    return 0;
}
int MP_StopAutoPlayTask()
{
    if(task_id>0)
    {
        MP_taskDelete(task_id);
        task_id = -1;
        MP_ClearAllPlayList();
        if(gAtiveMediaid>0)
            MP_stop(gAtiveMediaid);
        return 0;
    }
    
    return -1;

}

int MP_InActiveMediaID(int mediaID)
{
    //MP_PLIST_LOCK();
    log_info("inacitve media id\n");
    if(mediaID == gAtiveMediaid)
        gAtiveMediaid = -1;
    //MP_PLIST_UNLOCK();    
    return 0;
}

int MP_ActiveMediaID(int mediaID)
{
    //MP_PLIST_LOCK();
    log_info("acitve media id,media id is%d\n",mediaID);
    gAtiveMediaid = mediaID;
    //MP_PLIST_UNLOCK();    
    return 0;
    
}
int MP_PlayNextFile()
{   
    MP_FLIST_LOCK();
    if(gTotalItemsInFList >1)
        MP_stop(gAtiveMediaid);
    MP_FLIST_UNLOCK();
    
}
//=========================================

static void* MP_AutoPlayTask(void* args)
{     
    MP_mediaItem *item = NULL;    
    
    while(1)
    {
        //MP_PLIST_LOCK();           
        
        if(gAtiveMediaid <0)
        {
            
            item = MP_GetItemFromList(g_mode);   
            if((NULL !=item)&&(NULL !=item->url))
            {
                log_con("autoplay next item,item index:%d\n",item->index);
                gAtiveMediaid = MP_play_file(item->url,1,0,0,0,0);    
                free(item->url);
                free(item);
                //MP_PLIST_UNLOCK();
                usleep(100000);//100ms
            }

        }
        else
        {
            usleep(100000);//100ms
            //MP_PLIST_UNLOCK();

        }
    };
    return NULL;
}


