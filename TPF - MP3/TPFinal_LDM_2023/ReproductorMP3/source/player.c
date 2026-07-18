/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "player.h"
#include "fsl_dac.h"
#include "fsl_pit.h"
#include "fsl_edma.h"
#include "fsl_dac.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "sdmmc_config.h"
#include "fsl_sysmpu.h"
#include "MP3Dec.h"
#include "MK64F12.h"
#include "peripherals.h"
#include "board.h"
#include "pin_mux.h"
#include "equalizer.h"
#include "vumeter.h"
#include "eventQueue.h"
#include <stdio.h>
#include <string.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum _SDStates
{
    SDCard_ready,
    no_SDCard
} SDStates_t;

typedef enum _PlayerStates
{
    STARTING,
    SD_DETECTED,
    READING_SD,
    DECODING,
    WAITING_TO_DECODE,
    FILTERING,
    STOPPED,
    PAUSED,

} PlayerStates_t;

#define OUTBUFF_SIZE 1152
/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/
// /* Source address extern definition */
// extern uint16_t *srcAddrPing;
// /* Source address extern definition */
// extern uint16_t *srcAddrPong;
// /* Transactional transfer configuration */
// extern edma_transfer_config_t DMA_CH0_PING_config;
// /* Transactional transfer configuration */
// extern edma_transfer_config_t DMA_CH0_PONG_config;
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
/*
Function for reading all the mp3 files in the SD card in all folders and adding them to the song list
*/
static void readMP3Files();

void doNothing(void);

void processSamples(int16_t *buff, uint32_t buffSize, float samprate);
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static FATFS g_fileSystem; /* File system object */

static SDStates_t SDstate = SDCard_ready;
static PlayerStates_t state = STARTING;
static PlayerStates_t pausePrevState = WAITING_TO_DECODE;
static FRESULT error;

static float volumePlayer = 1;

static bool decodelag = false;

static song_node_t *currentSong = NULL;

int16_t pingBuffer[1152] = {[0 ... 1151] = 2048};
int16_t pongBuffer[1152] = {[0 ... 1151] = 2048};
static int16_t *currBuffPlaying = pingBuffer;

static uint32_t pingSampleRate = 44100;
static uint32_t pongSampleRate = 44100;

static const TCHAR driverNumberBuffer[4U] = {SDDISK + '0', ':', '/', '\0'};
bool failedFlag = false;
bool playingMusic = false;

char ch = '0';
BYTE work[FF_MAX_SS];

gpio_pin_config_t config =
    {
        kGPIO_DigitalOutput,
        0,
};
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool playerInit()
{
    BOARD_BootClockRUN();
    BOARD_InitPeripherals();
    BOARD_InitPins();
    BOARD_InitDebugConsole();
    SYSMPU_Enable(SYSMPU, false);
    initList();
    MP3DecInit();
    PORT_SetPinMux(PORTC, 3, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOC, 3, &config);

    BOARD_SD_Config(&g_sd, NULL, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);

    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        printf("\r\nSD host init fail\r\n");
        return false;
    }

    state = STARTING;
    return true;
}

bool playPlayer()
{
	if (SDstate != SDCard_ready)
	{
		return false;
	}
    switch (state)
    {
    case STOPPED:
        if (currentSong == NULL)
        {
            return true;
        }
        MP3SelectSong(currentSong->data);

        EDMA_AbortTransfer(&DMA_CH3_Handle);
        uint32_t decodedSamples = MP3DecDecode(pingBuffer, &pingSampleRate);
        PIT_SetTimerPeriod(PIT, kPIT_Chnl_3, PIT_CLK_FREQ / pingSampleRate);
        // Filtrado, casteo a 12 bits y correccion de offset
        processSamples(pingBuffer, decodedSamples, pingSampleRate);
        DMA_CH0_PING_config.majorLoopCounts = decodedSamples;
        EDMA_SubmitTransfer(&DMA_CH3_Handle, &DMA_CH0_PING_config);
        currBuffPlaying = pingBuffer;
        EDMA_StartTransfer(&DMA_CH3_Handle);
        PIT_StartTimer(PIT, kPIT_Chnl_3);

        state = DECODING;
        break;
    case PAUSED:
        PIT_StartTimer(PIT, kPIT_Chnl_3);
        state = pausePrevState;
        break;
    default:
        return false;
        break;
    }
    return false;
}

bool stopPlayer()
{
	if (SDstate != SDCard_ready)
	{
		return false;
	}
    PIT_StopTimer(PIT, kPIT_Chnl_3);
    DAC0->DAT[0].DATH = 0x8U; // Pull to middle
    DAC0->DAT[0].DATL = 0x00U;
    turnOffVumeter();
    state = STOPPED;
    return false;
}

bool pausePlayer()
{
	if (SDstate != SDCard_ready)
	{
		return false;
	}
    PIT_StopTimer(PIT, kPIT_Chnl_3);
    DAC0->DAT[0].DATH = 0x8U; // Pull to middle
    DAC0->DAT[0].DATL = 0x00U;
    pausePrevState = state;
    turnOffVumeter();
    state = PAUSED;
    return false;
}

bool nextSong()
{
	if (SDstate != SDCard_ready)
	{
		return false;
	}
    switch (state)
    {
    case DECODING:
    case WAITING_TO_DECODE:
        stopPlayer();
        if (getListTail() == currentSong->next)
        {
            currentSong = getListHead()->next;
        }
        else
        {
            currentSong = currentSong->next;
        }

        playPlayer();
        break;
    case STOPPED:
    case PAUSED:
        stopPlayer();
        if (getListTail() == currentSong->next)
        {
            currentSong = getListHead()->next;
        }
        else
        {
            currentSong = currentSong->next;
        }
        break;
    default:
        stopPlayer();
        if (getListTail() == currentSong->next)
        {
            currentSong = getListHead()->next;
        }
        else
        {
            currentSong = currentSong->next;
        }
        break;
    }

    return false;
}

bool prevSong()
{
	if (SDstate != SDCard_ready)
	{
		return false;
	}
    switch (state)
    {
    case DECODING:
    case WAITING_TO_DECODE:
        stopPlayer();
        if (getListHead() == currentSong->prev)
        {
            currentSong = getListTail()->prev;
        }
        else
        {
            currentSong = currentSong->prev;
        }

        playPlayer();
        break;
    case STOPPED:
    case PAUSED:
        stopPlayer();
        if (getListHead() == currentSong->prev)
        {
            currentSong = getListTail()->prev;
        }
        else
        {
            currentSong = currentSong->prev;
        }
        break;
    default:
        stopPlayer();
        if (getListHead() == currentSong->prev)
        {
            currentSong = getListTail()->prev;
        }
        else
        {
            currentSong = currentSong->prev;
        }
        break;
    }
    return false;
}

void adjustVolume(float vol)
{
    volumePlayer = vol;
}

song_node_t *getSongList()
{
    return getListHead();
}

bool selectSong(song_node_t *song)
{
    switch (state)
    {
    case STOPPED:
        currentSong = song;
        MP3SelectSong(currentSong->data);
        return false;
        break;

    default:
        currentSong = song;
        MP3SelectSong(currentSong->data);
        return false;
        break;
    }

    return true;
}

void getSongInfo(song_node_t *node);

player_msg_t updatePlayer()
{
    static uint32_t decodedSamples = 0;

    if (SDstate == SDCard_ready && SD_IsCardPresent(&g_sd) == false)
    {
        stopPlayer();
        putEvent(SDCardRemoved);
        SDstate = no_SDCard;
        state = STARTING;
    }

    switch (state)
    {
    case STARTING:

        /* wait card insert */
        if (SD_IsCardPresent(&g_sd) == true)
        {
            printf("\r\nCard inserted.\r\n");
            /* power off card */
            SD_SetCardPower(&g_sd, false);
            /* power on the card */
            SD_SetCardPower(&g_sd, true);

            SDstate = SDCard_ready;
            state = SD_DETECTED;
        }
        break;

    case SD_DETECTED:
        if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
        {
            printf("Mount volume failed.\r\n");
            break;
        }

        error = f_chdrive((char const *)&driverNumberBuffer[0U]);
        if (error)
        {
            printf("Change drive failed.\r\n");
            break;
        }

        state = READING_SD;
        break;

    case READING_SD:
        // Read MP· files on SD
        readMP3Files("/");
        selectSong(getListHead()->next);
        putEvent(SDCardInserted);
        state = STOPPED;
        break;

    case DECODING:
        GPIO_PinWrite(GPIOC, 3, 1);

        if (currBuffPlaying == pingBuffer)
        {
            decodedSamples = MP3DecDecode(pongBuffer, &pongSampleRate);
            if (decodedSamples == 0)
            {
                nextSong();
                putEvent(AutoNextSong);
                return 0;
            }

            // Filtrado, casteo a 12 bits y correccion de offset
            processSamples(pongBuffer, decodedSamples, pongSampleRate);
            DMA_CH0_PONG_config.majorLoopCounts = decodedSamples;
            if (EDMA_GetChannelStatusFlags(DMA_CH3_Handle.base, DMA_CH3_Handle.channel) & kEDMA_DoneFlag)
			{
				decodelag = true;
			}
            EDMA_SubmitTransfer(&DMA_CH3_Handle, &DMA_CH0_PONG_config);
        }
        else
        {
            decodedSamples = MP3DecDecode(pingBuffer, &pingSampleRate);
            if (decodedSamples == 0)
            {
                nextSong();
                putEvent(AutoNextSong);
                return 0;
            }

            // Filtrado, casteo a 12 bits y correccion de offset
            processSamples(pingBuffer, decodedSamples, pingSampleRate);
            DMA_CH0_PING_config.majorLoopCounts = decodedSamples;
            if (EDMA_GetChannelStatusFlags(DMA_CH3_Handle.base, DMA_CH3_Handle.channel) & kEDMA_DoneFlag)
            {
            	decodelag = true;
            }
            EDMA_SubmitTransfer(&DMA_CH3_Handle, &DMA_CH0_PING_config);
        }
        if (!decodelag)
        {
            state = WAITING_TO_DECODE;
        }
        else
        {
        	decodelag = false;
        	if (currBuffPlaying == pingBuffer)
			{
				currBuffPlaying = pongBuffer;
			}
			else
			{
				currBuffPlaying = pingBuffer;
			}
        }

        GPIO_PinWrite(GPIOC, 3, 0);
        break;

    case WAITING_TO_DECODE:
        doNothing();
        break;

    case STOPPED:
        doNothing();
        break;

    case PAUSED:
        doNothing();
        break;

    default:
        break;
    }
    return false;
}

char *getCurrentSongName()
{
    return currentSong->data->filename;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* DMA channel DMA_CH3 callback function */
void DMA_callback(edma_handle_t *handle, void *data, bool transferDone, uint32_t tcdNum)
{

    if (currBuffPlaying == pingBuffer)
    {
        currBuffPlaying = pongBuffer;
        PIT_SetTimerPeriod(PIT, kPIT_Chnl_3, PIT_CLK_FREQ / pongSampleRate);
    }
    else
    {
        currBuffPlaying = pingBuffer;
        PIT_SetTimerPeriod(PIT, kPIT_Chnl_3, PIT_CLK_FREQ / pingSampleRate);
    }

    state = DECODING;
}

void doNothing(void)
{
    return;
}

static void readMP3Files(const char *path)
{
    static char currentPath[256];
    static FILINFO fileInfo;
    DIR directory;

    FRESULT result = f_opendir(&directory, path); // Open the directory
    if (result != FR_OK)
    {
#if DEBUG==1
        printf("Error opening directory: %d\n", result);
#endif
        return;
    }

    while (1)
    {
        result = f_readdir(&directory, &fileInfo); // Read directory entries
        if (result != FR_OK || fileInfo.fname[0] == 0)
        {
            break; // Break on error or end of directory
        }

        if (fileInfo.fattrib & AM_DIR)
        {
            // If it's a directory, skip "." and ".."
            if (strcmp(fileInfo.fname, ".") == 0 || strcmp(fileInfo.fname, "..") == 0)
            {
                continue;
            }

            // Skip directories starting with a dot
            if (fileInfo.fname[0] == '.')
            {
                continue;
            }
            sprintf(currentPath, "%s/%s", path, fileInfo.fname);
            readMP3Files(currentPath);
        }
        else
        {
            // If it's a file, check for ".mp3" extension
            if (strstr(fileInfo.fname, ".mp3") != NULL)
            {
#if DEBUG==1
                printf("MP3 File: %s/%s\n", path, fileInfo.fname);
#endif
                // Add the file to the list
                pushtoBack(newSong(path, fileInfo.fname));
            }
        }
    }

    f_closedir(&directory); // Close the directory
}

void processSamples(int16_t *buff, uint32_t buffSize, float samprate)
{
    static float floatSamples[1152];
    static float floatSamplesaux[1152];
    static uint8_t vumeterDataout[8];

    for (size_t i = 0; i < buffSize; i++)
    {

        floatSamplesaux[i] = (float)buff[i] / 16.0f * volumePlayer;
    }

    processEqualizer(floatSamplesaux, floatSamples, buffSize);

    for (size_t i = 0; i < buffSize; i++)
    {
        floatSamplesaux[i] = floatSamples[i];
        floatSamples[i] = floatSamples[i] + 2048.0f;

        if (floatSamples[i] < 0)
        {
            buff[i] = 0;
        }
        else if (floatSamples[i] > 4095)
        {
            buff[i] = 4095;
        }
        else
        {
            buff[i] = (uint16_t)floatSamples[i];
        }
    }
    analizeBlock(floatSamplesaux, buffSize, samprate);
    analisis2vumeter(vumeterDataout);

    for (size_t i = 0; i < 8; i++)
    {
        selectBar(7 - i);
        setLevel(vumeterDataout[i]);
    }
}
