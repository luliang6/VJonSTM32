/**
  ******************************************************************************
  * @file    app_recorder.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Audio functional recorder driver.
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
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include "app_recorder.h"
#include "file_utils.h"
#include "mem_utils.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RecoderState_TypeDef  RecState = REC_STATE_IDLE;
extern FIL AudioFile;
uint32_t InBuffToken = 0;

xSemaphoreHandle                     RecorderTask_Sem = NULL;
portBASE_TYPE                        RecorderTaskWoken = pdFALSE;

/* Buffers */
uint16_t *pInBuff1 = NULL;
uint16_t *pInBuff2 = NULL;
uint16_t *pEncSamples = NULL;
uint32_t PcketSize = MAX_IN_PACKET_SZE;
uint32_t BuffReady = 0;
/* Buffer used to store the audio file header */
uint8_t tEncHeaderTmp[50];

uint32_t nbrEncData = 0;
uint32_t TLength = 0;
uint32_t WaveClose = 0;
uint32_t WrtNbr = 0;
static uint32_t SampleFreq = 16000;
/* Global structure for the encoder */
Encoder_TypeDef  sEncoderStruct;

/* Audio Recorder task variables */
xTaskHandle  AudioRecorder_Task_Handle = NULL;
xQueueHandle* pRecorderQueue;

/* Private function prototypes -----------------------------------------------*/
static uint32_t AudioRecorder_MonoToStereo(int16_t* BIn, int16_t* BOut, uint32_t Size);
void AudioRecorder_XferCplt(uint8_t Direction, uint8_t** pbuf, uint32_t* pSize);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialization of the audio recorder
  * @param  SamplingFreq: audio frequency to be set for the recorder
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_Init(uint32_t SamplingFreq)
{
  /* Initialize the recorder state */
  RecState = REC_STATE_IDLE;

  /* Reset the encoder structure */
  sEncoderStruct.EncoderInit = NULL;
  sEncoderStruct.EncoderDeInit = NULL;
  sEncoderStruct.EncoderStopMode = NULL;
  sEncoderStruct.Encoder_EncodeData = NULL;
  sEncoderStruct.Encoder_SetTags = NULL;
  sEncoderStruct.Encoder_GetSampleRate = NULL;
  sEncoderStruct.Encoder_GetStreamLength = NULL;
  sEncoderStruct.Encoder_GetElapsedTime = NULL;

  /* Initialize the Audio Input Hardware layer */
  if (AUDIO_IN_fops.Init(SamplingFreq, DEFAULT_VOLUME, 0) != 0)
  {
    return 2;
  }

  /* Set the callback to be called when a buffer transfer complete occurs */
  AUDIO_IN_fops.SetXferCpltCallback(AudioRecorder_XferCplt);

  SampleFreq = SamplingFreq;

  return 0;
}

/**
  * @brief  p_free all used resources
  * @param  None
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_DeInit(void)
{
  RecState = REC_STATE_IDLE;

  /* Free resources used by the Audio Input Hardware layer */
  if (AUDIO_IN_fops.DeInit(0) != 0)
  {
    return 2;
  }

  return 0;
}

/**
  * @brief  Open a file for recording
  * @param  FilePath: path to the file used for recording
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_Open(uint8_t  *FilePath)
{
  /* Check the current state */
  if (RecState != REC_STATE_IDLE)
  {
    return 1;
  }

  /* Open the specified file for writing */
  if(f_open(&AudioFile, (char *)FilePath, FA_CREATE_NEW | FA_WRITE) != FR_OK)
  {
    return 1;
  }

  /* Prepare buffer allocation */
  pInBuff1 = (uint16_t*)p_malloc((PcketSize/2)*sizeof(uint16_t));
  pInBuff2 = (uint16_t*)p_malloc((PcketSize/2)*sizeof(uint16_t));

  /* Check result of allocation */
  if ((pInBuff1 == NULL) || (pInBuff2 == NULL))
  {
    return 2;
  }


    /* Create semaphore */
  if (RecorderTask_Sem  == NULL)
  {
    vSemaphoreCreateBinary( RecorderTask_Sem );
  }

  if (AudioRecorder_Task_Handle == NULL)
  {
    /* Create and start the audio player task */
    xTaskCreate(AudioRecorder_Task, 
                (signed char const*)"RECORD_P", 
                RECORDER_TASK_STACK_SZE,
                NULL, 
                AudioRecorder_Task_PRIO, 
                &AudioRecorder_Task_Handle);
  }

  /* Select the appropriate encoder */
  Encoders_SelectEncoder(&sEncoderStruct, FilePath[strlen((char *)FilePath) - 1]);

  /* Initialize the encoder */
  sEncoderStruct.EncoderInit(SampleFreq, tEncHeaderTmp);

  /* Get the most appropriate packet size */
  switch (FilePath[strlen((char *)FilePath) - 1])
  {
    case 'V':
    case 'v':
      PcketSize = WAVIN_PACKET_SZE;
      /* Write the header file  */
      f_write(&AudioFile, tEncHeaderTmp, 44, (void *)&WrtNbr);
      WaveClose = 1;
      break;

    default:
      PcketSize = WAVIN_PACKET_SZE;
      break;
  }



  /* Update current state */
  RecState = REC_STATE_READY;

  return 0;
}

/**
  * @brief  Close the opened recorder file
  * @param  None
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_Close(void)
{

  if(sEncoderStruct.Encoder_GetStreamLength != NULL)
  {
    /* Deinitialize the encoder */
    TLength = sEncoderStruct.Encoder_GetStreamLength();
  }

  if(sEncoderStruct.EncoderDeInit != NULL)
  {
    /* Free resources used by the encoder */
    sEncoderStruct.EncoderDeInit();
  }

  if(RecorderTask_Sem !=NULL)
  {
    vQueueDelete (RecorderTask_Sem);
    RecorderTask_Sem = NULL;;
  }

  /* Kill the main Recorder task */
  if (AudioRecorder_Task_Handle != NULL)
  {
    vTaskDelete(AudioRecorder_Task_Handle);
    AudioRecorder_Task_Handle = NULL;
  }

  /* Write back the data length to the recorded audio file (applies only to WAV files) */
  if (WaveClose)
  {
    f_lseek(&AudioFile, 4);
    f_write(&AudioFile, (uint8_t*)&TLength, 1, (void *)&WrtNbr);
    f_write(&AudioFile, (uint8_t*)((uint32_t)&TLength + 1), 1, (void *)&WrtNbr);
    f_write(&AudioFile, (uint8_t*)((uint32_t)&TLength + 2), 1, (void *)&WrtNbr);
    f_write(&AudioFile, (uint8_t*)((uint32_t)&TLength + 3), 1, (void *)&WrtNbr);
    f_lseek(&AudioFile, 40);
    f_write(&AudioFile, (uint8_t*)&TLength, 1, (void *)&WrtNbr);
    f_write(&AudioFile, (uint8_t*)((uint32_t)&TLength + 1), 1, (void *)&WrtNbr);
    f_write(&AudioFile, (uint8_t*)((uint32_t)&TLength + 2), 1, (void *)&WrtNbr);
    f_write(&AudioFile, (uint8_t*)((uint32_t)&TLength + 3), 1, (void *)&WrtNbr);
    WaveClose = 0;
  }

  /* Free local buffers */
  if (pInBuff1 != NULL)
  {
    p_free (pInBuff1);
    pInBuff1 = NULL;
  }
  if (pInBuff2 != NULL)
  {
    p_free (pInBuff2);
    pInBuff2 = NULL;
  }

  /* Close the current file */
  f_close(&AudioFile);
    
  /* Update current state */
  RecState = REC_STATE_IDLE;

  return 0;
}

/**
  * @brief  Start recording into file
  * @param  None
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_Start(void)
{
  if (RecState == REC_STATE_READY)
  {
    /* Update current state */
    RecState = REC_STATE_RECORDING;

    /* Initialize token flag */
    InBuffToken = 0;

    /* Start recording received packet */
    AUDIO_IN_fops.AudioCmd((uint8_t*)pInBuff1,    /* Samples buffer pointer */
                           PcketSize/2,           /* Size of samples in Bytes */
                           AUDIO_IN_CMD_START);   /* Command to be processed */
  }

  return 0;
}

/**
  * @brief  Stop recording
  * @param  None
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_Stop(void)
{
  if (RecState == REC_STATE_RECORDING)
  {
    /* Update current state */
    RecState = REC_STATE_PAUSED;

    /* Stop recording */
    AUDIO_IN_fops.AudioCmd((uint8_t*)NULL,       /* Samples buffer pointer */
                           0,                    /* Number of samples in Bytes */
                           AUDIO_IN_CMD_STOP);   /* Command to be processed */
  }

  return 0;
}

/**
  * @brief  Pause or resume the audio recording
  * @param  Cmd: command, could be 0 to pause, !0 to resume
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_PauseResume(uint32_t Cmd)
{
  if (RecState == REC_STATE_RECORDING)
  {
    /* Update current state */
    RecState = REC_STATE_PAUSED;

    /* Pause recording */
    AudioRecorder_Stop();
  }
  else if (RecState == REC_STATE_PAUSED)
  {
    /* Update current state */
    RecState = REC_STATE_RECORDING;

    /* Resume recording */
    AudioRecorder_Start();
  }

  return 0;
}

/**
  * @brief  Main Audio Recorder Task
  * @param  pvParameters: RTOS parameters list
  * @retval None
*/
void AudioRecorder_Task(void * pvParameters)
{
  /* Start by obtaining the semaphore.*/
  xSemaphoreTake(RecorderTask_Sem, (portTickType ) 0);

  for ( ;; )
  {
    /* Check if a buffer need to be prepared */
    if (xSemaphoreTake(RecorderTask_Sem, (portTickType ) 100) == pdTRUE)
    { 

      if (RecState == REC_STATE_RECORDING)
      {

        /* Transform the Mono buffer to stereo.
        This operation may be avoided by using the feature: Stereo into the recorder, but this feature
        has not been tested yet. */
        AudioRecorder_MonoToStereo((int16_t*)((BuffReady==1)?  (uint8_t*)pInBuff1:(uint8_t*)pInBuff2), \
          (int16_t*)((BuffReady==1)?  (uint8_t*)pInBuff1:(uint8_t*)pInBuff2), \
            PcketSize/4);

        /* Set the number of PCM data */
        nbrEncData = PcketSize;

        /* Encode the recorded buffer */
        sEncoderStruct.Encoder_EncodeData((int8_t*)((BuffReady==1)? pInBuff1:pInBuff2), 
                                          (int8_t*)&pEncSamples, 
                                          &nbrEncData, 
                                          NULL);

        BuffReady = 0;

        /* Write the encoded buffer into audio file */
        f_write(&AudioFile, pEncSamples, (pEncSamples != NULL)? nbrEncData:0, (void *)&WrtNbr);
      }
    }
  }
} 

/**
  * @brief  Function called when a recorded buffer is ready, prepare the next.
  * @param  pbuf: pointer to the next buffer the recorder should use
  * @param  pSize: pointer to the size of the next buffer to be recorded
  * @retval !0 if operation failed.
  */
void AudioRecorder_XferCplt(uint8_t Direction, uint8_t** pbuf, uint32_t* pSize)
{
  /* Double buffering: token allows to select which buffer to use */

  /* Update the pointer to next buffer and its size */
  *pSize = PcketSize/2;
  *pbuf = (InBuffToken == 0)?  (uint8_t*)pInBuff2:(uint8_t*)pInBuff1;

  /* Toggle the buffer ready flag */
  BuffReady = (InBuffToken == 0)? 1:2;

  /* Toggle the token flag */
  InBuffToken = (InBuffToken == 0)? 1:0;

  /* Set semaphore to re-activate the audio player tasks that reads and decodes the next buffer */
  if (RecorderTask_Sem  != NULL)
  {
    xSemaphoreGiveFromISR( RecorderTask_Sem, &RecorderTaskWoken );

    if( RecorderTaskWoken != pdFALSE )
    {
      vPortYieldFromISR();
    }
  }

}


/**
  * @brief  Sets the audio file pointer position.
  * @param  pos: New position of the pointer to be set
  * @param  pSize: pointer to the size of the next buffer to be recorded
  * @retval !0 if operation failed.
  */
uint32_t AudioRecorder_SetPosition(uint32_t Pos)
{
  /* Call the Fat FS seek function */
  return f_lseek(&AudioFile, Pos);
}


/**
  * @brief  Pause or resume the audio recording
  * @param  Cmd: command, could be 1 to pause, 0 to resume
  * @retval 0: Passed, !0: Failed
  */
uint32_t AudioRecorder_GetElapsedTime(void)
{
  if ((RecState == REC_STATE_READY) || \
      (RecState == REC_STATE_RECORDING) || \
      (RecState == REC_STATE_PAUSED))
  {
    /* Get elapsed time */
    if (sEncoderStruct.Encoder_GetElapsedTime != NULL)
    {
      return sEncoderStruct.Encoder_GetElapsedTime();
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 0;
  }
}

/**
* @brief  Transforms a 16-bit mono buffer to a 16-bit stereo buffer by duplicating all values
* @param  BIn    : pointer to the input frame
* @param  BOut   : pointer to the output encoded frame (can be the same input buffer)
* @param  Size   : Input buffer frame size
* @retval 0 if OK.
*/
static uint32_t AudioRecorder_MonoToStereo(int16_t* BIn, int16_t* BOut, uint32_t Size)
{
  int32_t i;

  for (i = (Size*2)-1; i >= 0; i -= 2)
  {
    BOut[i] = BIn[i/2];
    BOut[i - 1] = BIn[i/2];
  }

  return 0;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
