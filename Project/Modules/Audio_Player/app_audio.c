/**
  ******************************************************************************
  * @file    app_audio.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Audio functional player driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied
  * See the License for the specific language governing permissions and
  * limitations under the License
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include "app_audio.h"
#include "file_utils.h"
#include "mem_utils.h"

/** @addtogroup STM32_Audio_Utilities
  * @{
  */


/** @defgroup STM32_AUDIO_PLAYER
  * @brief STM32 Audio Player module
  * @{
  */

/** @defgroup STM32_AUDIO_PLAYER_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup STM32_AUDIO_PLAYER_Private_Defines
  * @{
  */
#define INFO_MAX_LENGTH                    19
/**
  * @}
  */


/** @defgroup STM32_AUDIO_PLAYER_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup STM32_AUDIO_PLAYER_Private_Variables
  * @{
  */
/* Main Buffer for Audio Data Out transfers and its relative pointers */
typedef struct
{
  uint16_t* pBuf;
  uint32_t  Size;
  uint32_t  isReady;
}AUDIO_Buffer_TypeDef;

AUDIO_Buffer_TypeDef AudioBuffers[3];

uint32_t OutPacketSize = MAX_OUT_PACKET_SZE;
uint16_t *OutBuff1 = NULL, *OutBuff2 = NULL, *OutBuff3 = NULL;
uint32_t WrBuffIdx = 0, RdBuffIdx = 0;


/* Current decoded buffer size */
uint32_t      tsize = 0;

FIL AudioFile;
__IO uint32_t isFileInfoAvailable = 0;

 /* Buffer used to store the audio file header */
uint8_t tHeaderTmp[MAX_AUDIO_HEADER_SIZE];
__IO uint32_t NumberOfData = 0;

__IO uint32_t SmallPacketCount = 0;

extern PLAYER_SettingsTypeDef AudioPlayerSettings;


xTaskHandle  AudioPlayer_Task_Handle= NULL;

static AudioPlayerState_TypeDef AudioPlayerState = PLAYER_IDLE;

TAGS_TypeDef* pTrackInfo;

char iTitle  [MAX_TAG_STRING_LENGTH + 1];
char iArtist [MAX_TAG_STRING_LENGTH + 1];
char iAlbum  [MAX_TAG_STRING_LENGTH + 1];
char iYear   [MAX_TAG_STRING_LENGTH + 1];
char iGenre  [MAX_TAG_STRING_LENGTH + 1];

TAGS_TypeDef iTrackInfo = {
  iTitle,
  iArtist,
  iAlbum,
  iYear,
  iGenre,
  0
};

static uint32_t      SamplingRate = 0;
/* Decoder structure instance */
Decoder_TypeDef sDecoderStruct;


xSemaphoreHandle                     PlayerTask_Sem = NULL;
portBASE_TYPE                        PlayerTaskWoken = pdFALSE;
/**
  * @}
  */


/** @defgroup STM32_AUDIO_PLAYER_Private_FunctionPrototypes
  * @{
  */
void  AudioPlayer_XferCplt (uint8_t Direction, uint8_t** pbuf, uint32_t* pSize);
static uint32_t Player_FRead (void* pfile, uint8_t* pbuf, uint32_t size);
static  void AudioPlayer_DeInitBuffer(void);
/* static uint32_t AudioPlayer_MonoToStereo(int16_t* BIn, int16_t* BOut, uint32_t Size); */
/**
  * @}
  */

/******************************************************************************/
#ifndef __DISABLE_EQUALIZER_CTRL
/*****************************  Equalizer  ************************************/
__IO uint32_t  EqInstance[SPIRIT_EQ_PERSIST_SIZE_IN_BYTES/4]  ;
TSpiritEQ_Band EQ_Bands[SPIRIT_EQ_MAX_BANDS];
/******************************************************************************/
#endif

#ifndef __DISABLE_LOUDNESS_CTRL
/**************************  Loudness Control *********************************/
__IO uint32_t LdCtrlPersistance[SPIRIT_LDCTRL_PERSIST_SIZE_IN_BYTES/4];
uint32_t LdCtrlScratch[SPIRIT_LDCTRL_SCRATCH_SIZE_IN_BYTES/4];
TSpiritLdCtrl_Prms LdCtrlInstanceParams;
/******************************************************************************/
#endif

/** @defgroup STM32_AUDIO_PLAYER_Private_Functions
  * @{
  */

/**
  * @brief  Initialize all resources used by the Audio Player
  * @param  None
  * @retval None
  */
uint32_t AudioPlayer_Init(void)
{
  /* Set the default state of the player to idle */
  AudioPlayerState = PLAYER_IDLE;
  isFileInfoAvailable = 0;

  /* Set the track info to default values */
  strcpy(iTrackInfo.Title,  "    ");
  strcpy(iTrackInfo.Artist, "    ");
  strcpy(iTrackInfo.Album,  "    ");
  strcpy(iTrackInfo.Year,   "    ");
  strcpy(iTrackInfo.Genre,  "    ");
  iTrackInfo.SamplingRate = 0;

  /* Initialize the decoder instance with Null pointers */
  Decoders_SelectDecoder(&sDecoderStruct, 'v');

  /* Set packet size to the default value */
  OutPacketSize = MAX_OUT_PACKET_SZE;

  /* Allocate max packet size to avoid fragmentation */
  OutBuff1 =(uint16_t*) p_malloc(MAX_OUT_PACKET_SZE);
  OutBuff2 =(uint16_t*) p_malloc(MAX_OUT_PACKET_SZE);
  OutBuff3 =(uint16_t*) p_malloc(MAX_OUT_PACKET_SZE);
  if ((OutBuff1 == NULL) || (OutBuff2 == NULL) || (OutBuff3 == NULL))
  {
    p_free (OutBuff1);
    p_free (OutBuff2);
    p_free (OutBuff3);
    return 1;
  }

  /* Create semaphore */
  if (PlayerTask_Sem  == NULL)
  {
    vSemaphoreCreateBinary( PlayerTask_Sem );
  }

  if (AudioPlayer_Task_Handle == NULL)
  {
    /* Create and start the audio player task */
    xTaskCreate(AudioPlayer_Task, 
                (signed char const*)"AUDIO_P", 
                PLAYER_TASK_STACK_SZE,
                NULL,
                PLAYER_TASK_PRIO,
                &AudioPlayer_Task_Handle);
  }

  return 0;
}

/**
  * @brief  free all resources used by the Audio Player
  * @param  None
  * @retval None
  */
uint32_t AudioPlayer_DeInit(void)
{
  /* Set the default state of the player to idle */
  AudioPlayerState = PLAYER_IDLE;

  /* Kill the main player task */
  if(PlayerTask_Sem !=NULL)
  {
    vQueueDelete (PlayerTask_Sem);
    PlayerTask_Sem = NULL;
  }

  if (AudioPlayer_Task_Handle != NULL)
  {
    vTaskDelete(AudioPlayer_Task_Handle);
    AudioPlayer_Task_Handle = NULL;
  }

  /* p_free the audio buffer */
  if (OutBuff1 != NULL)
  {
    p_free(OutBuff1);
    OutBuff1 = NULL;
  }
  if (OutBuff2 != NULL)
  {
    p_free(OutBuff2);
    OutBuff2 = NULL;
  }
  if (OutBuff3 != NULL)
  {
    p_free(OutBuff3);
    OutBuff3 = NULL;
  }

  WrBuffIdx = RdBuffIdx = 0;

  return 0;
}

/**
  * @brief  Starts a new session of audio playing (variables initializations...)
  * @param  FilePath : Audio file full path.
  *         volume   : initial volume 
  * @retval Status
  */
uint32_t AudioPlayer_Open(uint8_t  *FilePath, uint16_t volume)
{


  char temp[129];
#ifndef __DISABLE_EQUALIZER_CTRL
  __IO uint32_t index;
  TSpiritEQ_Band *tmpEqBand;
#endif
#ifndef __DISABLE_LOUDNESS_CTRL
  __IO uint32_t ldness_perc_value;
#endif

  /* Initialization of track information structure */
  strcpy(iTrackInfo.Title,  "unknown");
  strcpy(iTrackInfo.Artist, "unknown");
  strcpy(iTrackInfo.Album,  "unknown");
  strcpy(iTrackInfo.Year,   "unknown");
  strcpy(iTrackInfo.Genre,  "unknown");
  iTrackInfo.SamplingRate = 0;

  /* Check player state */
  if (AudioPlayerState != PLAYER_IDLE)
  {
    return 1;
  }

  /* Open the audio file */
  if(f_open(&AudioFile, (char *)FilePath, FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
    return 1;
  }

  /* Get the header buffer from the audio File */
  f_read(&AudioFile, tHeaderTmp, MAX_AUDIO_HEADER_SIZE, (void *)&NumberOfData);

  /* Select the appropriate decoder */
  Decoders_SelectDecoder(&sDecoderStruct, FilePath[strlen((char *)FilePath) - 1]);

  /* Use the most suitable packet size */
  OutPacketSize = sDecoderStruct.PacketSize;

  /* Initialization of the decoder */
  if (sDecoderStruct.DecoderInit != NULL)
  {
    

      
      
    if (sDecoderStruct.DecoderInit((uint8_t*)tHeaderTmp, 
                                   Dec_ReadDataCallback, 
                                   AudioPlayer_SetPosition) != 0)
    {
      return 2;
    }
  }

  /* Extract the stream tags if available */
  if (sDecoderStruct.Decoder_ReadTags != NULL)
  {
    sDecoderStruct.Decoder_ReadTags(&iTrackInfo, &AudioFile);
  }
  else
  {
    GetFileOnly(temp, (char *)FilePath);

    if (strlen((char *)temp) > INFO_MAX_LENGTH)
    {
      (temp)[INFO_MAX_LENGTH - 2] = '.';
      (temp)[INFO_MAX_LENGTH - 1] = '.';
      (temp)[INFO_MAX_LENGTH] = '\0';
    }
    
    strcpy(iTrackInfo.Title, temp);
  }

  /* Extract the current sampling rate */
  if (sDecoderStruct.Decoder_GetSamplingRate != NULL)
  {
    SamplingRate = sDecoderStruct.Decoder_GetSamplingRate();
  }

  iTrackInfo.SamplingRate = SamplingRate;

  isFileInfoAvailable = 1;

  /* Initialize the Audio output Hardware layer */
  if (AUDIO_OUT_fops.Init(SamplingRate, volume, 0) != 0)
  {
    return 3;
  }

  /* Set the callback to be called when a buffer transfer complete occurs */
  AUDIO_OUT_fops.SetXferCpltCallback(AudioPlayer_XferCplt);

#ifndef __DISABLE_EQUALIZER_CTRL
  /* Enable the CRC */
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
  CRC->CR = CRC_CR_RESET;
  for (int index=0; index<300; index++);

  /* Enable the Eq */
  SpiritEQ_Init((TSpiritEq *)EqInstance, SamplingRate);

  /* Retrieve stored settings and set band params */
  SpiritEQ_FltGet((TSpiritEq *)EqInstance, &EQ_Bands[0], 0 );
  MOD_GetParam(AUDIO_EQU1_MEM , (uint32_t *)&EQ_Bands[0].gainDb);
  SET_BAND_PRMS(&EQ_Bands[0], SPIRIT_EQ_FLT_TYPE_SHELVING_LOWPASS , 0, 1000, EQ_Bands[0].gainDb);
  SpiritEQ_FltGet((TSpiritEq *)EqInstance, &EQ_Bands[1], 1 );
  MOD_GetParam(AUDIO_EQU2_MEM , (uint32_t *)&EQ_Bands[1].gainDb);
  SET_BAND_PRMS(&EQ_Bands[1], SPIRIT_EQ_FLT_TYPE_PEAKING , 3000, 2000, EQ_Bands[1].gainDb);
  SpiritEQ_FltGet((TSpiritEq *)EqInstance, &EQ_Bands[2], 2 );
  MOD_GetParam(AUDIO_EQU3_MEM , (uint32_t *)&EQ_Bands[2].gainDb);
  SET_BAND_PRMS(&EQ_Bands[3], SPIRIT_EQ_FLT_TYPE_PEAKING , 12000, 6000, EQ_Bands[2].gainDb);
  SpiritEQ_FltGet((TSpiritEq *)EqInstance, &EQ_Bands[3], 3 );
  MOD_GetParam(AUDIO_EQU4_MEM , (uint32_t *)&EQ_Bands[3].gainDb);
  SET_BAND_PRMS(&EQ_Bands[4], SPIRIT_EQ_FLT_TYPE_SHELVING_HIPASS , 18000, 2000, EQ_Bands[3].gainDb);

  for (index = 0; index < SPIRIT_EQ_MAX_BANDS ; index++)
  {
    tmpEqBand = &EQ_Bands[index];
    SpiritEQ_FltSet((TSpiritEq *)EqInstance, tmpEqBand, index);
  }
#endif /* __DISABLE_EQUALIZER */

#ifndef __DISABLE_LOUDNESS_CTRL
  /* Enable the CRC */
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
  CRC->CR = CRC_CR_RESET;
  for (int index=0; index<300; index++);

  /* Enable Loudness Control */
  SpiritLdCtrl_Init((TSpiritLdCtrl*)LdCtrlPersistance, SamplingRate);
  SpiritLdCtrl_GetPrms((TSpiritLdCtrl*)LdCtrlPersistance, &LdCtrlInstanceParams);
  MOD_GetParam(AUDIO_LDNS_MEM , (uint32_t *)&ldness_perc_value);
  LdCtrlInstanceParams.gainQ8 = PERC_TO_LDNS_DB(ldness_perc_value);
  SpiritLdCtrl_SetPrms((TSpiritLdCtrl*)LdCtrlPersistance, &LdCtrlInstanceParams);
#endif /* __DISABLE_LOUDNESS_CTRL */

  return 0;
}

/**
  * @brief  Closes the current session of audio playing (p_free resources).
  * @param  None
  * @retval Status
  */
uint32_t AudioPlayer_Close(void)
{

  AudioPlayerState = PLAYER_IDLE;

  /* DeInitialize the Audio output Hardware layer */
  if (AUDIO_OUT_fops.DeInit(0) != 0)
  {
    return 2;
  }

  /* Close the decoder instance */
  if (sDecoderStruct.DecoderDeInit != NULL)
  {
    sDecoderStruct.DecoderDeInit(); 
  }

  /* Empty the decoder structure */
  Decoders_SelectDecoder(&sDecoderStruct, ' ');

  /* Close the current file */
  f_close(&AudioFile);

  isFileInfoAvailable = 0;

  return 0;
}

/**
  * @brief  Update state and pointers. This function is called when previous buffer
  *         has been consumed and the next buffer should be prepared (read from
  *         storage).  
  * @param  None
  * @retval Status
  */

uint32_t AudioPlayer_PlayUpdate(void)
{
  /* Check if the buffer has already been used */
  if ((AudioBuffers[WrBuffIdx].isReady == 0))
  {
    /* Read and decode next buffer */
    AudioBuffers[WrBuffIdx].Size = Player_FRead (NULL,
                                                 (uint8_t*)AudioBuffers[WrBuffIdx].pBuf,
                                                 OutPacketSize);
    AudioBuffers[WrBuffIdx].isReady = 1;

    /* Update index */
    WrBuffIdx = (WrBuffIdx >= 2)? 0:(WrBuffIdx+1);
  }
 
  return 0;
}


/**
  * @brief  Main audio player task.
  * @param  pvParameters not used
  * @retval None
  */
void AudioPlayer_Task(void * pvParameters)
{

  /* Start by obtaining the semaphore.*/
  xSemaphoreTake(PlayerTask_Sem, (portTickType ) 0);

  for ( ;; )
  {
    /* Check if a buffer need to be prepared */
    if (xSemaphoreTake(PlayerTask_Sem, (portTickType ) 50) == pdTRUE)
    { 

      if (AudioPlayerState == PLAYER_PLAYING)
      {
        /* Get next buffer from mass storage device and decode it */
        AudioPlayer_PlayUpdate();
      }
    }
  }
}

/**
  * @brief  Start playing the audio file.
  * @param  FilePath : Audio file full path.
  *         volume   : initial volume 
  * @retval Status
  */
uint32_t AudioPlayer_Play(uint8_t *FilePath, uint16_t volume)
{
  switch (AudioPlayerState)
  {
  case PLAYER_IDLE:
  case PLAYER_STOPPED:
    /* Open the audio file for read */
    if (AudioPlayer_Open(FilePath, volume) != 0)
    {
      return 1;
    }
    AudioPlayer_DeInitBuffer();

    /* Start the audio streaming */
    AUDIO_OUT_fops.AudioCmd((uint8_t*)AudioBuffers[0].pBuf,  /* Samples buffer pointer */
                            AudioBuffers[0].Size,  /* Number of samples in Bytes */
                            AUDIO_CMD_PLAY);       /* Command to be processed */

    /* Set the current state */
    AudioPlayerState = PLAYER_PLAYING;
    break;

  case PLAYER_PAUSED:
    /* Set the current state */
    AudioPlayerState = PLAYER_PLAYING;

    /* Resume the audio stream */
    AUDIO_OUT_fops.AudioCmd((uint8_t*)AudioBuffers[RdBuffIdx].pBuf,  /* Samples buffer pointer */
                            AudioBuffers[RdBuffIdx].Size,  /* Number of samples in Bytes */
                            AUDIO_CMD_PLAY);               /* Command to be processed */
    break;

  case PLAYER_ERROR:
  case PLAYER_PLAYING:
    break;
  }

  return 0;
}

/**
  * @brief  Pause the audio file streaming.
  * @param  None
  * @retval Status
  */
uint32_t AudioPlayer_Pause(void)
{
  switch (AudioPlayerState)
  {
  case PLAYER_IDLE:
  case PLAYER_PAUSED:
  case PLAYER_ERROR:
  case PLAYER_STOPPED:    
    break;

  case PLAYER_PLAYING:
    /* Set the current state to paused */
    AudioPlayerState = PLAYER_PAUSED;

    /* Pause the audio stream */
    AUDIO_OUT_fops.AudioCmd(0,                 /* Samples buffer pointer */
                            0,                 /* Number of samples in Bytes */
                            AUDIO_CMD_PAUSE);  /* Command to be processed */
    break;
  }

  return 0;
}

/**
  * @brief  Stops the audio file streaming.
  * @param  None
  * @retval Status
  */
uint32_t AudioPlayer_Stop(void)
{
  /* Set the current state */
  AudioPlayerState = PLAYER_STOPPED;

  /* Pause the audio stream */
  AUDIO_OUT_fops.AudioCmd(0,                /* Samples buffer pointer */
                          0,                /* Number of samples in Bytes */
                          AUDIO_CMD_PAUSE); /* Command to be processed */

  return 0;
}

/**
  * @brief  Forward the current position of audio file pointer.  
  * @param  None
  * @retval Status
  */
uint32_t AudioPlayer_Forward(void)
{
  return 0;
}

/**
  * @brief  Rewind the current position of audio file pointer.
  * @param  None
  * @retval None
  */
uint32_t AudioPlayer_Rewind(void)
{
  return 0;
}

/**
  * @brief  Manages the volume of the current played file.
  * @param  volume   : new volume value
  * @retval Status
  */
uint32_t AudioPlayer_VolumeCtrl(uint8_t Volume)
{
   /* Update the volume level */
   return AUDIO_OUT_fops.VolumeCtl(Volume);
}





/**
  * @brief  Updates the current track information.
  * @param  None.
  * @retval pointer to a track information structure to be updated
  */
TAGS_TypeDef* AudioPlayer_GetFileInfo(void)
{
  return (&iTrackInfo);
}


/**
  * @brief  Updates the current time information (expressed in seconds).
  * @param  Length: pointer to the variable containing the total audio track length
  * @param  Total: pointer to the variable containing the elapsed time
  * @retval 0 if Passed, !0 else.
  */
uint32_t AudioPlayer_GetTimeInfo(uint32_t* Length, uint32_t* Elapsed)
{
  if (sDecoderStruct.Decoder_GetStreamLength != NULL)
  {
    *Length = sDecoderStruct.Decoder_GetStreamLength(AudioPlayer_GetFileLength());
  }
  if (sDecoderStruct.Decoder_GetElapsedTime != NULL)
  {
    *Elapsed = sDecoderStruct.Decoder_GetElapsedTime(AudioFile.fptr);
  }

  return 0;
}



/**
  * @brief  Reads a number of data from the audio file (if the fils is compressed, 
  *         then the decompression algorithm function is called).
  * @param  pfile: pointer to the file.
  * @param  pbuf: Pointer to the address of the ram buffer to be filled
  * @param  size: number of data to be read in bytes
  * @retval number of data actually read (in bytes).
  */
static uint32_t  Player_FRead (void* pfile, uint8_t* pbuf, uint32_t size)
{
  /* Initialize the total size variable */
  tsize = 0;

  /* Call the low layer function to get raw data */ 
  if (sDecoderStruct.Decoder_DecodeData != NULL)
  {
    tsize = sDecoderStruct.Decoder_DecodeData((__IO int16_t*)pbuf, (size/4), NULL);
  }

  /* If there is no data to read, exit player */
  if (tsize == 0)
  {
    /* Check on the end of file */
    AudioPlayer_Stop();
    return 0;
  }

#ifndef __DISABLE_EQUALIZER_CTRL
  if (AudioPlayerSettings.isEqDisabled == 0) 
  {
    /* Apply Equalizer settings */
    SpiritEQ_Apply((void *)EqInstance,
                   /* NB_Channel */2, 
                   (int16_t *)(pbuf),
                   tsize / 4);
  }
#endif /* __DISABLE_EQUALIZER_CTRL */


#ifndef __DISABLE_LOUDNESS_CTRL
    /* Apply Loudness settings */
  if (AudioPlayerSettings.isEqDisabled == 0)
  {
    if (LdCtrlScratch != NULL)
    {
      SpiritLdCtrl_Apply((TSpiritLdCtrl *)LdCtrlPersistance,
                         /* NB_Channel */ 2, 
                         (int16_t *)(pbuf),
                         tsize  / 4, 
                         (void *)LdCtrlScratch);
    }
    else
    {
      CONSOLE_LOG((uint8_t *)"[ERR] Audio Module: Cannot alloc enough memory for Loudness control");
    }
  }
#endif /* __DISABLE_LOUDNESS_CTRL */
  
  return tsize;
}

/**
  * @brief  Manage end of buffer transfer for each device.
  * @param  Direction: could be DIRECTION_IN or DIRECTION_OUT
  * @param  pbuf: Pointer to the address of the current buffer
  * @param  pSize: pointer to the variable which holds current buffer size
  * @retval status
  */
void  AudioPlayer_XferCplt (uint8_t Direction, uint8_t** pbuf, uint32_t* pSize)
{
  if (AudioPlayerState == PLAYER_PLAYING)
  {
    /* Release previous buffer for write operation */
    AudioBuffers[RdBuffIdx].isReady = 0;

    /* Increment the buffer index */
    RdBuffIdx = (RdBuffIdx >= 2)? 0:(RdBuffIdx+1);

    if (!(AudioBuffers[RdBuffIdx].isReady))
    {
      RdBuffIdx = (RdBuffIdx > 0)? (RdBuffIdx-1):2;
    }
    /* Resume the audio stream */
    AUDIO_OUT_fops.AudioCmd((uint8_t*)AudioBuffers[RdBuffIdx].pBuf,  /* Samples buffer pointer */
                            AudioBuffers[RdBuffIdx].Size,  /* Number of samples in Bytes */
                            AUDIO_CMD_PLAY);               /* Command to be processed */

    /* Set semaphore to reactivate the audio player tasks that reads and decodes the next buffer */
    if (PlayerTask_Sem  != NULL)
    {
      xSemaphoreGiveFromISR( PlayerTask_Sem, &PlayerTaskWoken );

      if( PlayerTaskWoken != pdFALSE )
      {
        vPortYieldFromISR();
      }
    }
  }
}

/**
  * @brief  Sets the current position of the audio file pointer.
  * @param  Pos: pointer position to be set
  * @retval Status
  */
uint32_t AudioPlayer_SetPosition(uint32_t Pos)
{
  /* Call the Fat FS seek function */
  return f_lseek(&AudioFile, Pos);
}

/**
  * @brief  Sets the current position of the audio file pointer.
  * @param  Pos: pointer position to be set
  * @retval Status
  */
uint32_t AudioPlayer_GetFileLength(void)
{
  /* Call the Fat FS seek function */
  return AudioFile.fsize;
}

/**
  * @brief  Returns the current state of the audio player.
  * @param  None
  * @retval PLAYER_IDLE, PLAYER_PLAYING, PLAYER_PAUSED, PLAYER_STOPPED or PLAYER_ERROR.
  */
AudioPlayerState_TypeDef AudioPlayer_GetState(void)
{
  /* Return the current state of the audio player */
  return AudioPlayerState;
}


/**
  * @brief  set the new state of the audio player.
* @param  new state : PLAYER_IDLE, PLAYER_PLAYING, PLAYER_PAUSED, PLAYER_STOPPED or PLAYER_ERROR.
  * @retval None
  */
void AudioPlayer_SetState(AudioPlayerState_TypeDef state)
{
  /* set the new state of the audio player */
   AudioPlayerState = state;
}
/**
  * @brief  DeInit the audio player buffers.
  * @param  None
  * @retval None.
*/    
static  void AudioPlayer_DeInitBuffer(void)
{
  AudioBuffers[0].pBuf = OutBuff1;
  AudioBuffers[1].pBuf = OutBuff2;
  AudioBuffers[2].pBuf = OutBuff3;

  for (int i = 0; i < MAX_NUM_BUFF; i++)
  {
    AudioBuffers[i].Size = MAX_OUT_PACKET_SZE;
    AudioBuffers[i].isReady = 0;
  }


  WrBuffIdx = RdBuffIdx = 0;

  /* Fill all buffers with data */
  for (int i = 0; i<3; i++)
  {
    AudioPlayer_PlayUpdate();
  }
}  
/**
  * @brief  Callback function to supply the decoder with input MP3 bitstreaam.
  * @param  pMP3CompressedData: pointer to the target buffer to be filled.
  * @param  nMP3DataSizeInChars: number of data to be read in bytes
  * @param  pUserData: optional parameter (not used in this version)
  * @retval return the mp3 decode frame.
  */
uint32_t Dec_ReadDataCallback(void *    pCompressedData,        /* [OUT] Bitbuffer */
                                  uint32_t  nDataSizeInChars, /* sizeof(Bitbuffer) */
                                  void *    pUserData               /* Application-supplied parameter */
)
{
 FRESULT res;
 res = f_read(&AudioFile, pCompressedData, nDataSizeInChars, (void *)&NumberOfData);
 
 if((res != FR_OK) || //case of error
    (NumberOfData == 0) || // no data
    (AudioFile.fptr >= AudioFile.fsize))// end of file
 {
   AudioPlayer_Stop();
 }

  return  NumberOfData;
}


/******************************************************************************
     AUDIO correction mechanism management
******************************************************************************/
/**
  * @brief  Get the current shift and direction values between input and output streams.
  * @param  pValue: value of the shift to be corrected.
  * @param  pDirection: direction of the correction (0:overrun or 1:underrun)
  * @param  pBuff: Option pointer
  * @retval 0: there are available data to process. !0: no available data to process.
  */
uint32_t AUDIO_CORR_Out_GetCurrent(uint32_t* pValue, uint32_t* pDirection, uint32_t** pBuff)
{
  /*
     Not yet implemented...
   */
  return 0;
}

/**
  * @brief  Get the current shift and direction values between input and output streams.
  * @param  pValue: value of the shift to be corrected.
  * @param  pDirection: direction of the correction (0:overrun or 1:underrun)
  * @param  pBuff: Option pointer
  * @retval 0: there are available data to process. !0: no available data to process.
  */
uint32_t AUDIO_CORR_In_GetCurrent(uint32_t* pValue, uint32_t* pDirection, uint32_t** pBuff)
{
  /*
     Not yet implemented...
   */
  return 0;
}


/**
  * @brief  EVAL_AUDIO_TransferComplete_CallBack
  * @param  None
  * @retval None
  */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
}

/**
  * @brief  Codec_TIMEOUT_UserCallback
  * @param  None
  * @retval 0
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
  I2C_InitTypeDef I2C_InitStructure;

  I2C_GenerateSTOP(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, DISABLE);

  I2C_DeInit(I2C1);

  /* CODEC_I2C peripheral configuration */
  I2C_DeInit(I2C1);
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x33;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  /* Enable the I2C peripheral */
  I2C_Cmd(I2C1, ENABLE);
  I2C_Init(I2C1, &I2C_InitStructure);

  return 0;
}

/**
  * @brief  CLK_SYNTH_TIMEOUT_UserCallback
  * @param  None
  * @retval Status
  */
uint32_t CLK_SYNTH_TIMEOUT_UserCallback(void)
{
  I2C_InitTypeDef I2C_InitStructure;

  I2C_GenerateSTOP(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, ENABLE);
  I2C_SoftwareResetCmd(I2C1, DISABLE);

  I2C_DeInit(I2C1);

  /* CODEC_I2C peripheral configuration */
  I2C_DeInit(I2C1);
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x33;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  /* Enable the I2C peripheral */
  I2C_Cmd(I2C1, ENABLE);
  I2C_Init(I2C1, &I2C_InitStructure);

  return 0;
}

/**
  * @brief  STM32_AudioRec_DataProcessSingle
  *         This is an example of how to implement filtering or encoding functions
  *         into the audio recorder module. It simply increase volume by shifting data
  *         right. This function should process one single data at a time
  * @param  pdata: pointer to the data to be processed
  * @param  fmt: number of bits in the data sample (ie. 16 or 8)
  * @param  chnbr: number of channels in the audio stream. It could be 1: Mono or 2: Stereo
  * @retval 0 if all operations are OK. !0 else
  */
uint32_t STM32_AudioRec_DataProcessSingle(uint8_t* pdata, uint32_t fmt, uint32_t chnbr)
{

  return 0;
}

/**
  * @brief  STM32_AudioRec_DataProcessBuffer
  *         This is an example of how to implement filtering or encoding functions
  *         into the audio recorder module. It simply increase volume by shifting data
  *         right. This function should process one buffer at a time
  * @param  pbuf: pointer to the data buffer to be processed
  * @param  size: size of the data buffer to be processed (in bytes)
  * @param  fmt: number of bits in the data sample (ie. 16 or 8)
  * @param  chnbr: number of channels in the audio stream. It could be 1: Mono or 2: Stereo
  * @retval 0 if all operations are OK. !0 else
  */
uint32_t STM32_AudioRec_DataProcessBuffer(uint8_t * pbuf, uint32_t size, uint32_t fmt, uint32_t chnbr)
{

  return 0;
}



/**
  * @brief  This function Set the new gain of the equalizer
  * @param  BandNum : equalizer band index
  * @param  NewGainValue : the new band gain.
  * @retval None.
*/
void App_Audio_SetEq(uint8_t BandNum, int16_t NewGainValue)
{
#ifndef __DISABLE_EQUALIZER_CTRL
  /* GetBand */
  SpiritEQ_FltGet((TSpiritEq *)EqInstance, &EQ_Bands[BandNum], BandNum );

  /* SetNewValue */
  EQ_Bands[BandNum].gainDb = PERC_TO_EQUI_DB(NewGainValue);

  SpiritEQ_FltSet((TSpiritEq *)EqInstance, &EQ_Bands[BandNum], BandNum);
#endif /* __DISABLE_EQUALIZER_CTRL */
}


/**
  * @brief  This function Set Loudness Control Gain Value
  * @param  NewGainValue: the new band gain in %
  * @retval None.
  */
void App_Audio_SetLoudnessGain(int16_t NewGainValue)
{
#ifndef __DISABLE_LOUDNESS_CTRL
  /* Get old Gain */
  SpiritLdCtrl_GetPrms((TSpiritLdCtrl*)LdCtrlPersistance, &LdCtrlInstanceParams);
  LdCtrlInstanceParams.gainQ8 = PERC_TO_LDNS_DB (NewGainValue);
  SpiritLdCtrl_Reset  ((TSpiritLdCtrl*)LdCtrlPersistance);
  SpiritLdCtrl_SetPrms((TSpiritLdCtrl*)LdCtrlPersistance, &LdCtrlInstanceParams);
#endif /* __DISABLE_EQUALIZER_CTRL */
}
/**
  * @}
  */ 


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
