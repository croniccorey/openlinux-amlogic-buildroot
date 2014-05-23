#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp_utils.h"
#include "mp_log.h"

int MP_pack_command_msg(MP_CommandMsg* inMsg,unsigned char* outMsg,int* outLen)
{
    int len = -1;
    int cursor = 0;
    if(NULL == inMsg||NULL == outMsg)
    {
        log_err("Input parameter Must NOT NULL\n");
        return -1;
    }

    len = sizeof(inMsg->head)+inMsg->auxDatLen+4;
    *outLen = len;
    memcpy(outMsg,(void*)&inMsg->head,sizeof(inMsg->head));
    cursor += sizeof(inMsg->head);
    memcpy(outMsg+cursor,(void*)&inMsg->auxDatLen,sizeof(inMsg->auxDatLen));
    cursor += sizeof(inMsg->auxDatLen);
    
    /*
    if(NULL == inMsg->auxDat||inMsg->auxDatLen <4)
    {
        log_err("Auxiliary data MUST NOT NULL!\n");
        return -1;
    }
    */
    
    switch(inMsg->head.type)
    {
    case MP_CMD_PLAYFILE:
        if(inMsg->auxDatLen < 4*7 +1)
        {
            log_err("Auxiliary data MUST more than 25 for playefile!\n");
            return -1;
        }

        memcpy(outMsg+cursor,(void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->urlLength,4);
        cursor +=4;
        if(NULL != ((MP_PlayFileMsgBody*)inMsg->auxDat)->url)
        {
            memcpy(outMsg+cursor,\
               ((MP_PlayFileMsgBody*)inMsg->auxDat)->url,\
               ((MP_PlayFileMsgBody*)inMsg->auxDat)->urlLength);
        }

        cursor +=((MP_PlayFileMsgBody*)inMsg->auxDat)->urlLength;
        memcpy(outMsg+cursor,\
               (void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->isStart,4);
        cursor +=4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->isRepeat,4);
        cursor +=4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->isBlackout,4);
        cursor +=4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->stPosition,4);

        cursor +=4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->playDuration,4);
        cursor += sizeof(int);
        memcpy(outMsg+cursor,\
               (void*)&((MP_PlayFileMsgBody*)inMsg->auxDat)->plmode,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_PLAYLIST:
        break;
    case MP_CMD_PAUSE:
        memcpy(outMsg+cursor,inMsg->auxDat,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_RESUME:
        memcpy(outMsg+cursor,inMsg->auxDat,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_STOP:
        memcpy(outMsg+cursor,inMsg->auxDat,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_QUITPLAYER:
        memcpy(outMsg+cursor,inMsg->auxDat,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_FASTFORWARD:
        memcpy(outMsg+cursor,(void*)&((MP_TrickPlayMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_TrickPlayMsgBody*)inMsg->auxDat)->speed,4);
        cursor += 4;
        break;
    case MP_CMD_FASTREWIND:
        memcpy(outMsg+cursor,(void*)&((MP_TrickPlayMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_TrickPlayMsgBody*)inMsg->auxDat)->speed,4);
        cursor += 4;
        break;
    case MP_CMD_SEEK:
        memcpy(outMsg+cursor,(void*)&((MP_SeekMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_SeekMsgBody*)inMsg->auxDat)->isStart,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_SeekMsgBody*)inMsg->auxDat)->searchPos,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_MUTE:
        memcpy(outMsg+cursor,(void*)&((MP_MuteMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_MuteMsgBody*)inMsg->auxDat)->isMute,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_REPEAT:
        memcpy(outMsg+cursor,(void*)&((MP_RepeatMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_RepeatMsgBody*)inMsg->auxDat)->isRepeat,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_BLACKOUT:
        memcpy(outMsg+cursor,(void*)&((MP_BlackoutMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_BlackoutMsgBody*)inMsg->auxDat)->isBlackout,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_START:
        memcpy(outMsg+cursor,inMsg->auxDat,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_SETVOLUME:
        memcpy(outMsg+cursor,(void*)&((MP_SetVolumeMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_SetVolumeMsgBody*)inMsg->auxDat)->vol,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_SETTONE:
        memcpy(outMsg+cursor,(void*)&((MP_SetToneMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,(void*)&((MP_SetToneMsgBody*)inMsg->auxDat)->tone,4);
        cursor += sizeof(int);
        break;
    case MP_CMD_SETOPTION:
        break;
    case MP_CMD_SETAUDIOTRACK:
        memcpy(outMsg+cursor,(void*)&((MP_SetAudioTrackMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_SetAudioTrackMsgBody*)inMsg->auxDat)->track_pid\
               ,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_CMD_SETSUBTITLE:
        memcpy(outMsg+cursor,(void*)&((MP_SetSubtitleMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_SetSubtitleMsgBody*)inMsg->auxDat)->sub_pid\
               ,sizeof(int));
        cursor += sizeof(int);
        break;

    case MP_CMD_ADDSOURCE:
        if(NULL != inMsg->auxDat&& inMsg->auxDatLen > 0)
        {
            memcpy(outMsg+cursor,inMsg->auxDat,inMsg->auxDatLen);
            cursor+=inMsg->auxDatLen;
        }
        else
        {
            log_err("Add invalid media source to amplayer,drop it\n");            
            return -1;
        }
        break;
    case MP_CMD_GETVOLUME:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_CMD_SETSONGSPECTRUM:
        memcpy(outMsg+cursor,(void*)&((MP_SetSpectrumMsgBody*)inMsg->auxDat)->mediaID,4);
        cursor += 4;
        memcpy(outMsg+cursor,\
               (void*)&((MP_SetSpectrumMsgBody*)inMsg->auxDat)->isStartPop\
               ,sizeof(int));
        cursor += sizeof(int);
        memcpy(outMsg+cursor,\
            (void*)&((MP_SetSpectrumMsgBody*)inMsg->auxDat)->interval\
            ,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_CMD_QUERY:        
        break;
    case MP_CMD_GETAUDIOTRICK:        
    case MP_CMD_GETSUBTITLE:        
    case MP_CMD_GETMEDIAINFO:
        if(inMsg->auxDatLen >4)
        {
            memcpy(outMsg+cursor,(void*)&((MP_GetInfoMsgBody*)inMsg->auxDat)->mediaID,sizeof(int));
            cursor+=sizeof(int);            
            if(NULL != ((MP_GetInfoMsgBody*)inMsg->auxDat)->url)
           {
                memcpy(outMsg+cursor,\
                   ((MP_GetInfoMsgBody*)inMsg->auxDat)->url,\
                   inMsg->auxDatLen - sizeof(int));           
                
                
                cursor+= inMsg->auxDatLen - sizeof(int);

            }
        }
        else
        {
            memcpy(outMsg+cursor,(void*)&((MP_GetInfoMsgBody*)inMsg->auxDat)->mediaID,sizeof(int));
            cursor += sizeof(int);   
        }
        break;
    case MP_CMD_GETPOSITION:
    case MP_CMD_GETDURATION:        
    
    case MP_CMD_GETSTATUS:    
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_CMD_GETALLMEDIAID:
        log_info("Get all media id\n");
        break;
    default:
        log_err("failed to pack,never see this line\n");
        break;
    }

    if(cursor != len)
    {
        log_err("current cursor position:%d,total msg length:%d\n",cursor,len);
        return -1;

    }
    return 0;

}
int MP_unpack_command_msg(unsigned char* inMsg,unsigned int msgLen,MP_CommandMsg* outMsg,MP_CommandType* type)
{
    int cursor =0;
    void* auxDat = NULL;
    char* tmp = NULL;
    if(NULL == inMsg||NULL == outMsg)
    {
        log_err("Input parameter Must NOT NULL\n");
        return -1;
    }

    if(msgLen < sizeof(MP_CommandMsg) -4)
    {
        log_err("invalid command msg\n");
        return -1;
    }

    memcpy((void*)&outMsg->head,inMsg,sizeof(outMsg->head));
    cursor += sizeof(outMsg->head);
    memcpy((void*)&outMsg->auxDatLen,inMsg+cursor,4);
    cursor += 4;

    switch(outMsg->head.type)
    {
    case MP_CMD_PLAYFILE:
        if(msgLen < sizeof(MP_CommandMsg)+25)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",41);
            return -1;
        }
        auxDat = (MP_PlayFileMsgBody*)malloc(sizeof(MP_PlayFileMsgBody));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory for MP_PlayFileMsgBody");
            return -1;
        }
        memset(auxDat,0,sizeof(MP_PlayFileMsgBody));
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->urlLength,inMsg+cursor,4);

        cursor += 4;//url length;
        if(((MP_PlayFileMsgBody*)auxDat)->urlLength > 0)
        {
            ((MP_PlayFileMsgBody*)auxDat)->url = (char*)malloc(((MP_PlayFileMsgBody*)auxDat)->urlLength);
            if(NULL == ((MP_PlayFileMsgBody*)auxDat)->url)
            {
                free((MP_PlayFileMsgBody*)auxDat);
                auxDat = NULL;
                return -1;
            }
            memcpy(((MP_PlayFileMsgBody*)auxDat)->url,inMsg+cursor,((MP_PlayFileMsgBody*)auxDat)->urlLength);

        }
        else
        {            
            free((MP_PlayFileMsgBody*)auxDat);
            auxDat = NULL;
            return -1;
        }

        cursor += ((MP_PlayFileMsgBody*)auxDat)->urlLength;
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->isStart,inMsg+cursor,4);
        cursor += 4;
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->isRepeat,inMsg+cursor,4);
        cursor += 4;
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->isBlackout,inMsg+cursor,4);
        cursor += 4;
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->stPosition,inMsg+cursor,4);
        cursor += 4;
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->playDuration,inMsg+cursor,4);
        cursor += 4;
        memcpy((void*)&((MP_PlayFileMsgBody*)auxDat)->plmode,inMsg+cursor,4);
        cursor += 4;
        outMsg->auxDat = auxDat;

        *type = MP_CMD_PLAYFILE;
        break;
    case MP_CMD_PLAYLIST:
        break;
    case MP_CMD_PAUSE:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;

        *type = MP_CMD_PAUSE;
        break;
    case MP_CMD_RESUME:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        *type = MP_CMD_RESUME;
        break;
    case MP_CMD_STOP:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        *type = MP_CMD_STOP;
        break;
    case MP_CMD_QUITPLAYER:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        *type = MP_CMD_QUITPLAYER;
        break;
    case MP_CMD_ADDSOURCE:
        if(outMsg->auxDatLen>0&&outMsg->auxDatLen<=1024)
        {
            auxDat = (char*)malloc(outMsg->auxDatLen);
            if(auxDat == NULL)
            {
                log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                return -1;
            }

            memcpy(auxDat,inMsg+cursor,outMsg->auxDatLen);
            outMsg->auxDat = auxDat;
            *type = MP_CMD_ADDSOURCE;
        } 
        else
        {
           log_err("failed to unpack add media source msg\n");
           return -1;
        }
        break;
    case MP_CMD_START:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        *type = MP_CMD_START;
        break;
    case MP_CMD_FASTFORWARD:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        auxDat = (MP_TrickPlayMsgBody*)malloc(sizeof(MP_TrickPlayMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy((void*)&((MP_TrickPlayMsgBody*)auxDat)->mediaID,inMsg+cursor,4);
        cursor += 4;

        memcpy((void*)&((MP_TrickPlayMsgBody*)auxDat)->speed,inMsg+cursor,4);
        cursor += 4;
        outMsg->auxDat = auxDat;
        *type = MP_CMD_FASTFORWARD;
        break;
    case MP_CMD_FASTREWIND:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        
        auxDat = (MP_TrickPlayMsgBody*)malloc(sizeof(MP_TrickPlayMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy((void*)&((MP_TrickPlayMsgBody*)auxDat)->mediaID,inMsg+cursor,4);
        cursor += 4;

        memcpy((void*)&((MP_TrickPlayMsgBody*)auxDat)->speed,inMsg+cursor,4);
        cursor += 4;
        outMsg->auxDat = auxDat;
        *type = MP_CMD_FASTREWIND;
        break;
    case MP_CMD_MUTE:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        auxDat = (MP_MuteMsgBody*)malloc(sizeof(MP_MuteMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(MP_MuteMsgBody));        
        cursor += sizeof(MP_MuteMsgBody);
        outMsg->auxDat = auxDat;
        *type = MP_CMD_MUTE;
        break;
    case MP_CMD_REPEAT:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        auxDat = (MP_RepeatMsgBody*)malloc(sizeof(MP_RepeatMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }
        memcpy(auxDat,inMsg+cursor,sizeof(MP_RepeatMsgBody));   
        cursor +=sizeof(MP_RepeatMsgBody);
        outMsg->auxDat = auxDat;
        *type = MP_CMD_REPEAT;
        break;
    case MP_CMD_BLACKOUT:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        auxDat = (MP_BlackoutMsgBody*)malloc(sizeof(MP_BlackoutMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(MP_BlackoutMsgBody));
        cursor +=sizeof(MP_BlackoutMsgBody);
        outMsg->auxDat = auxDat;
        *type = MP_CMD_BLACKOUT;
        break;
    case MP_CMD_SETVOLUME:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        auxDat = (MP_SetVolumeMsgBody*)malloc(sizeof(MP_SetVolumeMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(MP_SetVolumeMsgBody));
        outMsg->auxDat = auxDat;
        cursor+= sizeof(MP_SetVolumeMsgBody);
        *type = MP_CMD_SETVOLUME;
        break;
    case MP_CMD_SETTONE:
        if(msgLen < sizeof(MP_CommandMsg)+4)//4
        {
            log_err("input msg payload less than %d,invalid msg!\n",24);
            return -1;
        }
        auxDat = (MP_SetToneMsgBody*)malloc(sizeof(MP_SetToneMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(MP_SetToneMsgBody));
        cursor +=sizeof(MP_SetToneMsgBody);
        outMsg->auxDat = auxDat;
        *type = MP_CMD_SETTONE;
        break;
    case MP_CMD_SEEK:
        if(msgLen < sizeof(MP_CommandMsg)+8)//8,sizeof(mediaID)+sieof(speed)-sizeof(void*)
        {
            log_err("input msg payload less than %d,invalid msg!\n",8);
            return -1;
        }
        auxDat = (MP_SeekMsgBody*)malloc(sizeof(MP_SeekMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(MP_SeekMsgBody));
        outMsg->auxDat = auxDat;
        cursor += sizeof(MP_SeekMsgBody);
        *type = MP_CMD_SEEK;
        break;
    case MP_CMD_SETOPTION:
        *type = MP_CMD_SETOPTION;
        break;
    case MP_CMD_SETAUDIOTRACK:
        if(msgLen < sizeof(MP_CommandMsg))
        {
            log_err("input msg payload less than %d,invalid msg!\n",12);
            return -1;
        }
        auxDat = (MP_SetAudioTrackMsgBody*)malloc(sizeof(MP_SetAudioTrackMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }
        memcpy(auxDat,inMsg+cursor,sizeof(MP_SetAudioTrackMsgBody));
        outMsg->auxDat = auxDat;
        cursor +=sizeof(MP_SetAudioTrackMsgBody);
        *type = MP_CMD_SETAUDIOTRACK;
        break;
    case MP_CMD_SETSUBTITLE:
        if(msgLen < sizeof(MP_CommandMsg))
        {
            log_err("input msg payload less than %d,invalid msg!\n",12);
            return -1;
        }
        auxDat = (MP_SetSubtitleMsgBody*)malloc(sizeof(MP_SetSubtitleMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }
        memcpy(auxDat,inMsg+cursor,sizeof(MP_SetSubtitleMsgBody));
        outMsg->auxDat = auxDat;
        cursor += sizeof(MP_SetSubtitleMsgBody);
        *type = MP_CMD_SETSUBTITLE;
        break;
    case MP_CMD_SETSONGSPECTRUM:
        if(msgLen < sizeof(MP_CommandMsg))
        {
            log_err("input msg payload less than %d,invalid msg!\n",12);
            return -1;
        }
        
        auxDat = (MP_SetSpectrumMsgBody*)malloc(sizeof(MP_SetSpectrumMsgBody));
        if(NULL == auxDat)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }
        memcpy(auxDat,inMsg+cursor,sizeof(MP_SetSpectrumMsgBody));
        outMsg->auxDat = auxDat;
        cursor += sizeof(MP_SetSpectrumMsgBody);
        *type = MP_CMD_SETSONGSPECTRUM;
        break;
    case MP_CMD_GETVOLUME:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        *type = MP_CMD_GETVOLUME;
        break;
    case MP_CMD_QUERY:
        *type = MP_CMD_QUERY;
        break;
    case MP_CMD_GETAUDIOTRICK:  

        if(outMsg->auxDatLen >4)
        {
            auxDat = (MP_GetInfoMsgBody*)malloc(sizeof(MP_GetInfoMsgBody));
            if(auxDat == NULL)
            {
                 log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                 return -1;   
            }
            memcpy((void*)&((MP_GetInfoMsgBody*)auxDat)->mediaID,inMsg+cursor,sizeof(int));
            cursor += sizeof(int);
            tmp = (char*)malloc(outMsg->auxDatLen - sizeof(int));
            if(NULL == tmp)
            {
                log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                free(auxDat);
                return -1;
            }
            memcpy(tmp,inMsg+cursor,outMsg->auxDatLen - sizeof(int));

           ((MP_GetInfoMsgBody*)auxDat)->url  = tmp;
           tmp  = NULL;
           outMsg->auxDat = auxDat;
           cursor += outMsg->auxDatLen - sizeof(int);
            
        }
        else
        {
            auxDat = (MP_GetInfoMsgBody*)malloc(sizeof(MP_GetInfoMsgBody));
            if(auxDat == NULL)
            {
                 log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                 return -1;   
            }
            memcpy((void*)&((MP_GetInfoMsgBody*)auxDat)->mediaID,inMsg+cursor,sizeof(int));
            cursor += sizeof(int);

            ((MP_GetInfoMsgBody*)auxDat)->url = NULL;                   
            outMsg->auxDat = auxDat;
        }            
       
        *type = MP_CMD_GETAUDIOTRICK;
        break;
    case MP_CMD_GETSUBTITLE:  
     
        if(outMsg->auxDatLen >4)
        {
            auxDat = (MP_GetInfoMsgBody*)malloc(sizeof(MP_GetInfoMsgBody));
            if(auxDat == NULL)
            {
                 log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                 return -1;   
            }
            memcpy((void*)&((MP_GetInfoMsgBody*)auxDat)->mediaID,inMsg+cursor,sizeof(int));
            cursor += sizeof(int);
            tmp = (char*)malloc(outMsg->auxDatLen - sizeof(int));
            if(NULL == tmp)
            {
                log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                free(auxDat);
                return -1;
            }
            memcpy(tmp,inMsg+cursor,outMsg->auxDatLen - sizeof(int));

            ((MP_GetInfoMsgBody*)auxDat)->url  = tmp;
            tmp  = NULL;
            outMsg->auxDat = auxDat;
            cursor += outMsg->auxDatLen - sizeof(int);
            
        }
        else
        {
            auxDat = (MP_GetInfoMsgBody*)malloc(sizeof(MP_GetInfoMsgBody));
            if(auxDat == NULL)
            {
                 log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                 return -1;   
            }
            memcpy((void*)&((MP_GetInfoMsgBody*)auxDat)->mediaID,inMsg+cursor,sizeof(int));
            cursor += sizeof(int);

            ((MP_GetInfoMsgBody*)auxDat)->url = NULL;                   
            outMsg->auxDat = auxDat;
        }            
        *type = MP_CMD_GETSUBTITLE;
        break;
    case MP_CMD_GETPOSITION:  
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        cursor+= sizeof(int);
        *type = MP_CMD_GETPOSITION;
        
        break;
    case MP_CMD_GETDURATION:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        cursor+= sizeof(int);
        *type = MP_CMD_GETDURATION;        
        break;
    case MP_CMD_GETMEDIAINFO:    
        if(outMsg->auxDatLen >4)
        {
            auxDat = (MP_GetInfoMsgBody*)malloc(sizeof(MP_GetInfoMsgBody));
            if(auxDat == NULL)
            {
                 log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                 return -1;   
            }
            memcpy((void*)&((MP_GetInfoMsgBody*)auxDat)->mediaID,inMsg+cursor,sizeof(int));
            cursor += sizeof(int);
            tmp = (char*)malloc(outMsg->auxDatLen - sizeof(int));
            if(NULL == tmp)
            {
                log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                free(auxDat);
                return -1;
            }
            memcpy(tmp,inMsg+cursor,(outMsg->auxDatLen - sizeof(int)));
            
           ((MP_GetInfoMsgBody*)auxDat)->url  = tmp;
           tmp  = NULL;
           outMsg->auxDat = auxDat;
           cursor += outMsg->auxDatLen - sizeof(int);
            
        }
        else
        {
            auxDat = (MP_GetInfoMsgBody*)malloc(sizeof(MP_GetInfoMsgBody));
            if(auxDat == NULL)
            {
                 log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
                 return -1;   
            }
            memcpy((void*)&((MP_GetInfoMsgBody*)auxDat)->mediaID,inMsg+cursor,sizeof(int));
            cursor += sizeof(int);

           ((MP_GetInfoMsgBody*)auxDat)->url = NULL;                   
            outMsg->auxDat = auxDat;
        }            
        *type = MP_CMD_GETMEDIAINFO;        
        break;
    case MP_CMD_GETSTATUS:
        auxDat = (int*)malloc(sizeof(int));
        if(auxDat == NULL)
        {
            log_err("failed to malloc memory:%s[%d]",__FUNCTION__,__LINE__);
            return -1;
        }

        memcpy(auxDat,inMsg+cursor,sizeof(int));
        outMsg->auxDat = auxDat;
        cursor+= sizeof(int);
         *type = MP_CMD_GETSTATUS;
        break;
    case MP_CMD_GETALLMEDIAID:
        *type = MP_CMD_GETALLMEDIAID;
        break;
    default:
        *type = MP_CMD_UNKOWN;
        break;

    }

    return 0;

}

int MP_pack_response_msg(MP_ResponseMsg* inMsg,unsigned char* outMsg,int* outLen)
{
    int len = -1;
    int cursor = 0;
    int count;
    if(NULL == inMsg||NULL == outMsg)
    {
        log_err("Input parameter Must NOT NULL\n");
        return -1;
    }

    len = sizeof(inMsg->head)+inMsg->auxDatLen+2*sizeof(int);
    *outLen = len;
    memcpy(outMsg,(void*)&inMsg->head,sizeof(inMsg->head));
    cursor += sizeof(inMsg->head);
    memcpy(outMsg+cursor,(void*)&inMsg->mediaID,sizeof(int));
    cursor += sizeof(int);
    memcpy(outMsg+cursor,(void*)&inMsg->auxDatLen,sizeof(int));
    cursor += sizeof(int);
    
    switch(inMsg->head.type)
    {
    case MP_RESP_COMMAND:        
        break;
    case MP_RESP_STATE_CHANGED:
        memcpy(outMsg+cursor,(void*)&((MP_StateChangedRespBody*)inMsg->auxDat)->state,sizeof(int));        
        cursor += sizeof(int);
        memcpy(outMsg+cursor,(void*)&((MP_StateChangedRespBody*)inMsg->auxDat)->isMuteOn,sizeof(int));        
        cursor += sizeof(int);
        memcpy(outMsg+cursor,(void*)&((MP_StateChangedRespBody*)inMsg->auxDat)->isBlackout,sizeof(int));        
        cursor += sizeof(int); 
        memcpy(outMsg+cursor,(void*)&((MP_StateChangedRespBody*)inMsg->auxDat)->isRepeat,sizeof(int));        
        cursor += sizeof(int);

        if(((MP_StateChangedRespBody*)inMsg->auxDat)->stateEx)
        {
            memcpy(outMsg+cursor,((MP_StateChangedRespBody*)inMsg->auxDat)->stateEx,inMsg->auxDatLen - 4*sizeof(int));
            cursor +=(inMsg->auxDatLen- 4*sizeof(int)) ;
        }
        break;
    case MP_RESP_TIME_CHANGED:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_RESP_DURATION:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_RESP_VOLUME:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_RESP_QUERY:
        break;
    case MP_RESP_SET_OPTION:
        break;
    case MP_RESP_BUFFERING_PORGRESS:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(int));
        cursor += sizeof(int);
        break;
    case MP_RESP_AUDIOTRCK:
        memcpy(outMsg+cursor,(void*)&((MP_AudioTrackRespBody*)inMsg->auxDat)->trackcount,sizeof(int));
        cursor += sizeof(int);
        count = ((MP_AudioTrackRespBody*)inMsg->auxDat)->trackcount;    
        log_info("current audio track count:%d\n",count);
        while(count > 0)
        {
            memcpy(outMsg+cursor,(void*)&((MP_AudioTrackRespBody*)inMsg->auxDat)->item[count-1],sizeof(MP_AudioTrackItem));
            cursor +=sizeof(MP_AudioTrackItem);
            count -- ;  

        }

        break;
    case MP_RESP_SUBTITLE:
        memcpy(outMsg+cursor,(void*)&((MP_SubTitleRespBody*)inMsg->auxDat)->subcount,sizeof(int));
        cursor += sizeof(int);
        count = ((MP_SubTitleRespBody*)inMsg->auxDat)->subcount;        
        log_info("Current subtitle count:%d\n",count);
        //just for debug every items info.
        while(count >= 0)
        {
            memcpy(outMsg+cursor,(void*)&((MP_SubTitleRespBody*)inMsg->auxDat)->item[count-1],sizeof(MP_SubTitleItem));
            cursor +=sizeof(MP_SubTitleItem);
            count -- ;           
        }
        break;
    case MP_RESP_MEDIAINFO:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(MP_AVMediaFileInfo)-4);
        cursor+= sizeof(MP_AVMediaFileInfo) -4;        
        if(((MP_AVMediaFileInfo*)inMsg->auxDat)->has_tag==1)
        {   
            strncpy(outMsg+cursor,(void*)&((MP_AVMediaFileInfo*)inMsg->auxDat)->taginfo->title,TITLE_LEN_MAX);
            cursor+=TITLE_LEN_MAX;
            strncpy(outMsg+cursor,(void*)&((MP_AVMediaFileInfo*)inMsg->auxDat)->taginfo->album,ALBUM_LEN_MAX);
            cursor+=ALBUM_LEN_MAX;
            strncpy(outMsg+cursor,(void*)&((MP_AVMediaFileInfo*)inMsg->auxDat)->taginfo->author,AUTHOR_LEN_MAX);
            cursor+=AUTHOR_LEN_MAX;
           
            cursor +=sizeof(MP_AudioTagInfo);   
            
            
        }
        break;
    case MP_RESP_VALIDMEDIAID:
        memcpy(outMsg+cursor,inMsg->auxDat,sizeof(MP_ValidMediaIDInfo));
        cursor+= sizeof(MP_ValidMediaIDInfo) ; 
        break;
    default:
        break;
    }

   

    return 0;
}

int MP_unpack_response_msg(unsigned char* inMsg,unsigned int msgLen,MP_ResponseMsg* outMsg,MP_ResponseType* type)
{
    int cursor = 0;
    int count = 0;

    if(NULL == inMsg||NULL == outMsg)
    {
        log_err("Input parameter Must NOT NULL\n");
        return -1;
    }

    if(msgLen < sizeof(MP_ResponseMsg) - 4)//maybe,never need extend data
    {
        log_err("invalid response msg,drop it\n");
        return -1;
    }

    memcpy((void*)&outMsg->head,inMsg,sizeof(outMsg->head));
    cursor += sizeof(outMsg->head);
    memcpy((void*)&outMsg->mediaID,inMsg+cursor,sizeof(int));
    cursor += sizeof(int);

    memcpy((void*)&outMsg->auxDatLen,inMsg+cursor,sizeof(int));
    cursor += sizeof(int);

    switch(outMsg->head.type)
    {
    case MP_RESP_COMMAND:
        *type = MP_RESP_COMMAND;
        break;
    case MP_RESP_STATE_CHANGED:
        outMsg->auxDat = (MP_StateChangedRespBody*)malloc(sizeof(MP_StateChangedRespBody));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            return -1;
        }
      
        memcpy((void*)&((MP_StateChangedRespBody*)outMsg->auxDat)->state,inMsg+cursor,sizeof(int));        
        cursor += sizeof(int);

        memcpy((void*)&((MP_StateChangedRespBody*)outMsg->auxDat)->isMuteOn,inMsg+cursor,sizeof(int));        
        cursor += sizeof(int);

        memcpy((void*)&((MP_StateChangedRespBody*)outMsg->auxDat)->isBlackout,inMsg+cursor,sizeof(int));        
        cursor += sizeof(int);

        memcpy((void*)&((MP_StateChangedRespBody*)outMsg->auxDat)->isRepeat,inMsg+cursor,sizeof(int));        
        cursor += sizeof(int);
        
        if(outMsg->auxDatLen >(sizeof(MP_StateChangedRespBody) -4))
        {
            ((MP_StateChangedRespBody*)outMsg->auxDat)->stateEx = (MP_ExtMsgCode*)malloc(sizeof(MP_ExtMsgCode));
            if(NULL == ((MP_StateChangedRespBody*)outMsg->auxDat)->stateEx)
            {
                log_err("[%s]failed to malloc memory for state extend message\n",__FUNCTION__);
                free(outMsg->auxDat);
                return -1;
            }
            memcpy(((MP_StateChangedRespBody*)outMsg->auxDat)->stateEx,inMsg+cursor,sizeof(MP_ExtMsgCode));
            cursor += sizeof(MP_ExtMsgCode);
        }
        else
            ((MP_StateChangedRespBody*)outMsg->auxDat)->stateEx = NULL;
            
        *type = MP_RESP_STATE_CHANGED;
        break;
    case MP_RESP_TIME_CHANGED:
        outMsg->auxDat = (int*)malloc(sizeof(int));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            return -1;
        }
        
        memcpy(outMsg->auxDat,inMsg+cursor,sizeof(int));
        *type = MP_RESP_TIME_CHANGED;
         cursor += sizeof(int);
        break;
    case MP_RESP_DURATION:
        outMsg->auxDat = (int*)malloc(sizeof(int));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            return -1;
        }
        memcpy(outMsg->auxDat,inMsg+cursor,sizeof(int));
        *type = MP_RESP_DURATION;
         cursor += sizeof(int);
        break;
    case MP_RESP_VOLUME:
        outMsg->auxDat = (int*)malloc(sizeof(int));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            return -1;
        }
        memcpy(outMsg->auxDat,inMsg+cursor,sizeof(int));
        cursor+= sizeof(int);
        *type = MP_RESP_VOLUME;
        break;
    case MP_RESP_QUERY:

        *type = MP_RESP_QUERY;
        break;
    case MP_RESP_SET_OPTION:
        *type = MP_RESP_SET_OPTION;
        break;
    case MP_RESP_BUFFERING_PORGRESS:
        outMsg->auxDat = (int*)malloc(sizeof(int));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            return -1;
        }
        memcpy(outMsg->auxDat,inMsg+cursor,sizeof(int));
        cursor+= sizeof(int);
        *type = MP_RESP_BUFFERING_PORGRESS;
        break;
    case MP_RESP_AUDIOTRCK:
        outMsg->auxDat = (MP_AudioTrackRespBody*)malloc(sizeof(MP_AudioTrackRespBody));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            return -1;
        }
        memcpy((void*)&((MP_AudioTrackRespBody*)outMsg->auxDat)->trackcount,inMsg+cursor,sizeof(int));
        count = ((MP_AudioTrackRespBody*)outMsg->auxDat)->trackcount;
        cursor += sizeof(int);
        log_info("Current audiotrack count:%d after unpack response msg\n",count);
        if(count > 0)
        {
            memcpy((void*)&((MP_AudioTrackRespBody*)outMsg->auxDat)->item,inMsg+cursor,count*sizeof(MP_AudioTrackItem));
            cursor += count*sizeof(MP_AudioTrackItem);  
            
        }        

        *type = MP_RESP_AUDIOTRCK;
        break;
    case MP_RESP_SUBTITLE:
        outMsg->auxDat =(MP_SubTitleRespBody*)malloc(sizeof(MP_SubTitleRespBody));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            outMsg->auxDat = NULL;
            return -1;
        }
        memcpy((void*)&((MP_SubTitleRespBody*)outMsg->auxDat)->subcount,inMsg+cursor,sizeof(int));

        count = ((MP_SubTitleRespBody*)outMsg->auxDat)->subcount;
        cursor += sizeof(int);
        log_info("Current subtitle count:%d after unpack response msg\n",count);
        if(count > 0)
        {
            memcpy((void*)&((MP_SubTitleRespBody*)outMsg->auxDat)->item,inMsg+cursor,count*sizeof(MP_SubTitleItem));
            cursor += count*sizeof(MP_SubTitleItem);           
        }  

        *type = MP_RESP_SUBTITLE;
        break;
    case MP_RESP_MEDIAINFO:
        outMsg->auxDat = (MP_AVMediaFileInfo*)malloc(sizeof(MP_AVMediaFileInfo));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            outMsg->auxDat = NULL;
            return -1;
        }

        memcpy(outMsg->auxDat,inMsg+cursor,sizeof(MP_AVMediaFileInfo) -4);

        cursor +=sizeof(MP_AVMediaFileInfo) -4;
        if(((MP_AVMediaFileInfo*)outMsg->auxDat)->has_tag ==1)
        {
            MP_AudioTagInfo* taginfo = NULL;
            taginfo = (MP_AudioTagInfo*)malloc(sizeof(MP_AudioTagInfo));
            if(NULL ==taginfo)
            {
                log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
                memset(outMsg,0,sizeof(MP_ResponseMsg));
                outMsg->auxDat = NULL;
                return -1;

            }
            
            memcpy(taginfo,inMsg+cursor,sizeof(MP_AudioTagInfo));
           
            ((MP_AVMediaFileInfo*)outMsg->auxDat)->taginfo = taginfo;
             
            cursor +=sizeof(MP_AudioTagInfo);        

            
        } 
        else
            ((MP_AVMediaFileInfo*)outMsg->auxDat)->taginfo = NULL;

        *type = MP_RESP_MEDIAINFO;
        
        break;
    case MP_RESP_VALIDMEDIAID:
        outMsg->auxDat=(MP_ValidMediaIDInfo*)malloc(sizeof(MP_ValidMediaIDInfo));
        if(NULL == outMsg->auxDat)
        {
            log_err("[%s]failed to malloc memory.line:%d\n",__FUNCTION__,__LINE__);
            memset(outMsg,0,sizeof(MP_ResponseMsg));
            outMsg->auxDat = NULL;
            return -1;
        }
        memcpy(outMsg->auxDat,inMsg+cursor,sizeof(MP_ValidMediaIDInfo));
        *type = MP_RESP_VALIDMEDIAID;
        break;
    default:
        log_err("[%s]Never uppack this msg\n",__FUNCTION__);
        break;
    }

    //printf("[%s]message length is %d before unpacking,current cursor:%d\n",__FUNCTION__,msgLen,cursor);
    return 0;

}


int MP_free_command_msg(MP_CommandMsg* inMsg)
{
    if(NULL ==inMsg)
    {
        log_err("%s:invalid msg\n",__FUNCTION__);
        return -1;
    }
    switch(inMsg->head.type)
    {
    case MP_CMD_PLAYFILE:   
        if(inMsg->auxDat == NULL)
        {
            log_err("invalid MP_PlayFileMsgBody,%s\n",__FUNCTION__);
            return -1;
        }
        free(((MP_PlayFileMsgBody*)inMsg->auxDat)->url);

        free(inMsg->auxDat); 
        
        break;
    case MP_CMD_PLAYLIST:
        break;
    case MP_CMD_PAUSE:  
    case MP_CMD_RESUME:       
    case MP_CMD_STOP:  
    case MP_CMD_QUITPLAYER:     
    case MP_CMD_ADDSOURCE:
    case MP_CMD_START:       
    case MP_CMD_FASTFORWARD:       
    case MP_CMD_FASTREWIND:        
    case MP_CMD_MUTE:
        
    case MP_CMD_REPEAT:
        
    case MP_CMD_BLACKOUT:
      
    case MP_CMD_SETVOLUME:      
    case MP_CMD_SETTONE:
        
    case MP_CMD_SEEK:
      
    case MP_CMD_SETOPTION:   
    case MP_CMD_SETAUDIOTRACK:
      
    case MP_CMD_SETSUBTITLE:
       
    case MP_CMD_SETSONGSPECTRUM:
        
    case MP_CMD_GETVOLUME:
       
    case MP_CMD_QUERY:  
        if(inMsg->auxDat == NULL)
        {
            log_err("invalid msg:%s[%d]\n",__FUNCTION__,__LINE__);
            return -1;
        }
        free(inMsg->auxDat);        
        break;
 
    case MP_CMD_GETAUDIOTRICK:  

    case MP_CMD_GETSUBTITLE:       
        if(inMsg->auxDatLen >4)
        {
            free(((MP_GetInfoMsgBody*)inMsg->auxDat)->url);
            
            free(inMsg->auxDat);
            
        }
        else
        {
            free(inMsg->auxDat);
        }    
        
        break;
    case MP_CMD_GETPOSITION:  
      
    case MP_CMD_GETDURATION:
        if(inMsg->auxDat == NULL)
        {
            log_err("invalid msg:%s[%d]\n",__FUNCTION__,__LINE__);
            return -1;
        }
        free(inMsg->auxDat);        
          
        break;
    case MP_CMD_GETMEDIAINFO:    
        if(inMsg->auxDatLen >4)
        {
            free(((MP_GetInfoMsgBody*)inMsg->auxDat)->url);
            
            free(inMsg->auxDat);
            
        }
        else
        {
            free(inMsg->auxDat);
        } 
              
        break;
    case MP_CMD_GETSTATUS:
        if(inMsg->auxDat == NULL)
        {
            log_err("invalid msg:%s[%d]\n",__FUNCTION__,__LINE__);
            return -1;
        }
        free(inMsg->auxDat);        
          
        break;        
    case MP_CMD_GETALLMEDIAID:        
        break;
    default:        
        break;

    }
    
    free(inMsg);
    inMsg = NULL;
    return 0;
}
int MP_free_response_msg(MP_ResponseMsg* inMsg)
{
    if(NULL == inMsg)
    {
        log_err("invalid response msg.%s\n",__FUNCTION__);
        return -1;
    }
    switch(inMsg->head.type)
    {
        case MP_RESP_COMMAND:        
            break;
        case MP_RESP_STATE_CHANGED:  
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {
                if(((MP_StateChangedRespBody*)inMsg->auxDat)->stateEx)
                {
                    free(((MP_StateChangedRespBody*)inMsg->auxDat)->stateEx);
                }
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;
        case MP_RESP_TIME_CHANGED:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;
        case MP_RESP_DURATION:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;         
        case MP_RESP_VOLUME:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;        
            
        case MP_RESP_QUERY:
            break;
        case MP_RESP_SET_OPTION:
            break;
        case MP_RESP_BUFFERING_PORGRESS:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;           
           
        case MP_RESP_AUDIOTRCK:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;    
        case MP_RESP_SUBTITLE:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;            
            
        case MP_RESP_MEDIAINFO:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {     
                if(((MP_AVMediaFileInfo*)inMsg->auxDat)->taginfo)
                    free(((MP_AVMediaFileInfo*)inMsg->auxDat)->taginfo);
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;             
        case MP_RESP_VALIDMEDIAID:
            if((MP_StateChangedRespBody*)inMsg->auxDat)
            {             
                free((MP_StateChangedRespBody*)inMsg->auxDat);
            }
            break;   
        default:
            break;
        }      
        free(inMsg);
        inMsg = NULL;
        return 0;
    }
   
