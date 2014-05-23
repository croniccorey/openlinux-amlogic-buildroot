#ifndef MP_API_H
#define MP_API_H

#include"mp_types.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FILE_URL_MAX 1024



//======================================================================
//asynchronous API


/*
Description: add media source,but not start to play 
        if call this api,you must stop play.
        FSM: addsource->(start ->playing)->stop.
Comments:      
*/
int MP_AddMediaSource(const char* url);

/*
Description: Play media file, loop or not.
        @url:            play track's name,MAX length <1024
        @isStart:        if 0 , player wait start command, 1:play after decoded
        @isloop:         if 0 , do not loop; 1:loop
        @st:             if 0 , play from start,>0:play from st;
        @kp_duration:	 if 0 , ignore the duration; > 0: seconds to keep play.
        @plmode: all/videomono/audiomono
Comments: 
         20100618: isStart,st,kp_duration not ready.
         20100715: isStart ok. just can use MP_AddMediaSource as another option.
*/
int MP_play_file(const char*url,int isStart,int isLoop,int st,int kp_duration,MP_MediaPlayMode plmode);

/*
int MP_ClosePlayerInstance(int mediaID);
Description: from decoded to play

Comments:
        20100618: not ready.
        20100715 ok.
*/
int MP_start(int media_id);

/*
Description: from pause to play
*/
int MP_resume(int media_id);

/*
Description: from trickmode or play to pause
*/
int MP_pause(int media_id);

/*
Description:  1x, 2x, 4x, 8x forward.
*/
int MP_fastforward(int media_id,int speed);
/*
Description:  1x, 2x, 4x, 8x backward.
*/
int MP_rewind(int media_id,int speed);

/*
Description:  stop play
    @media_id:refer to play media returns.
Comments:
    if play another track,must stop current playback.
*/
int MP_stop(int media_id);

/*
Description:  seek a position to play
    @media_id:refer to play media returns.
    @pos: seek position
    @isStart: if 1 refer to start after seeking.
    
Comments:
    20100618: isStart not ready.
*/
int MP_seek(int media_id,int pos,int isStart);

int MP_mute(int media_id,int isMute);

int MP_SetVolume(int media_id,int vol);

int MP_GetVolume(int media_id);
/*
Description: set sound tone,just for koraok...
    @media_id:refer to play media returns.
    @tone: refer to left/right/all/swap  
*/
int MP_SetTone(int media_id,MP_Tone tone);

int MP_GetPosition(int media_id);

int MP_GetDuration(int media_id);

int MP_GetStatus(int media_id);

/*
Description: get internal subtitle enum.see mp_types.h 
        @ media_id: refer to play media returns.
Comments: 20100623:API function not ready.  
        20100713: must call MP_AddMediaSource,media_id must be valid.
*/
int MP_GetSubtitleInfo(const char*url,int media_id);
/*
Description: get audio track enum.see mp_types.h 
        @ media_id: refer to play media returns.
Comments: 20100623:API function not ready.    
        20100625:API function partly ok;
*/
int MP_GetAudioTrackInfo(const char*url,int media_id);

/*
Description: get media info,see mp_types.h 
        @ media_id: refer to play media returns.if url is ok, media id can be any number; 
Comments: 20100625:API function partly ok.        
*/
int MP_GetMediaInfo(const char*url,int media_id);


int MP_SetRepeat(int media_id,int isRepeat);

int MP_SetVideoBlackOut(int media_id,int isBlackout);

/*
Description: start or stop to pop subtitle raw data
        @ media_id: refer to play media returns.
        @ sub_uid: if 0xffff,refer to stop to POP raw data.others ok.
Comments:data path: /tmp/.subtitle_daemon_path    
        20100618:API function not ready.    
*/
int MP_SetInternalSubtitleOut(int media_id,int sub_uid);

/*
Description: start or stop to pop song spectrum data.
            @ media_id: refer to play media returns.
            @ isStatPop: if 0,stop to pop,1 means start.
            @ interval: if isStatPop is 1,will check this parameter. has default value.
*/
int MP_SetSongSepctralOut(int media_id,int isStartPop, int interval);

/*
Description: set MultiAudio to some audio track to play
        @ media_id: refer to play media returns.
        @ track_uid: can get from media info.
Comments:  
        20100618:API function not ready.    
*/
int MP_SetAudioTrack(int media_id,int track_uid);


//======================================================================
//synchronouce API,just for android adapter wrapper.

/*
Description: Get volume synchronoucely.
        @ media_id: refer to play media returns.
        @ vol: OUT value.   
        @ if 0 ok,others failed
*/
int MP_GetVolumeSync(int media_id,int* vol);
/*
Description: Get play position synchronoucely
        @ media_id: refer to play media returns.
        @ pos: OUT value.   
        @ if 0 ok,others failed
*/
int MP_GetPositionSync(int media_id,int* pos);
/*
Description: Get play status synchronoucely
        @ media_id: refer to play media returns.
        @ status: OUT value.   
        @isMuteon: OUT value,0 or 1(mute)
        @isRepeat: OUT value,0 or 1(repeat)
        @ if 0 ok,others failed
Comments: 
        20100618: isMuteon,isRepeat not ready.
*/
int MP_GetStatusSync(int media_id,int* status,int* isMuteon,int* isRepeat);

/*
Description: Get play duration synchronoucely
        @ media_id: refer to play media returns.
        @ dur: OUT value.   
        @ if 0 ok,others failed
*/
int MP_GetDurationSync(int media_id,int* dur);


//======================================================================
//some Miscs for testing,just can be as helpers.. 

int MP_TerminateAmplayerProcess(int media_id);

int MP_GetValidMediaIDSync(MP_ValidMediaIDInfo *mediaidpool);

#ifdef __cplusplus
}
#endif

#endif // MP_API_H
