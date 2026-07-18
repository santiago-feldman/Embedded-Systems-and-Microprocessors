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

void convert16to12(const int16_t *input, uint16_t *output, size_t numSamples);
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static FATFS g_fileSystem; /* File system object */

static PlayerStates_t state = STARTING;
static PlayerStates_t pausePrevState = WAITING_TO_DECODE;
static FRESULT error;

static song_node_t *currentSong = NULL;

int16_t pingBuffer[1152] = {[0 ... 1151] = 2048};
int16_t pongBuffer[1152] = {[0 ... 1151] = 2048};
static int16_t *currBuffPlaying = pingBuffer;

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
    PORT_SetPinMux(PORTB, 20, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOB, 20, &config);

    return true;
}

bool playPlayer()
{
    switch (state)
    {
    case STOPPED:
        if (currentSong == NULL)
        {
            return true;
        }
        MP3SelectSong(currentSong->data);

        EDMA_AbortTransfer(&DMA_CH3_Handle);
        uint32_t decodedSamples = MP3DecDecode(pingBuffer);

        // TODO: Aca va filtrado

        // 16bit to 12bit + offset correction
        convert16to12(pingBuffer, pingBuffer, decodedSamples);
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
    PIT_StopTimer(PIT, kPIT_Chnl_3);
    state = STOPPED;
    return false;
}

bool pausePlayer()
{
    PIT_StopTimer(PIT, kPIT_Chnl_3);
    pausePrevState = state;
    state = PAUSED;
    return false;
}

bool nextSong()
{
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
    return false;
}

bool prevSong()
{
    stopPlayer();
    if (getListHead() == currentSong->prev)
    {
        currentSong = getListTail();
    }
    else
    {
        currentSong = currentSong->prev;
    }

    playPlayer();
    return false;
}

bool adjustVolume(float vol);

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
    static uint32_t tickCount = 0;
    switch (state)
    {
    case STARTING:
        BOARD_SD_Config(&g_sd, NULL, BOARD_SDMMC_SD_HOST_IRQ_PRIORITY, NULL);

        /* SD host init function */
        if (SD_HostInit(&g_sd) != kStatus_Success)
        {
            printf("\r\nSD host init fail\r\n");
            break;
        }

        /* wait card insert */
        if (SD_IsCardPresent(&g_sd) == true)
        {
            printf("\r\nCard inserted.\r\n");
            /* power off card */
            SD_SetCardPower(&g_sd, false);
            /* power on the card */
            SD_SetCardPower(&g_sd, true);

            state = SD_DETECTED;
        }
        else
        {
            printf("\r\nCard detect fail.\r\n");
            break;
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
        readMP3Files((char const *)&driverNumberBuffer[0U]);
        selectSong(getListHead()->next);
        state = STOPPED;
        playPlayer();
        break;

    case DECODING:
        GPIO_PinWrite(GPIOB, 20, 1);

        if (currBuffPlaying == pingBuffer)
        {
            decodedSamples = MP3DecDecode(pongBuffer);
            if (decodedSamples == 0)
            {
                nextSong();
                return 0;
            }

            // TODO: Aca va filtrado
            // 16bit to 12bit + offset correction
            convert16to12(pongBuffer, pongBuffer, decodedSamples);
            DMA_CH0_PONG_config.majorLoopCounts = decodedSamples;
            EDMA_SubmitTransfer(&DMA_CH3_Handle, &DMA_CH0_PONG_config);
        }
        else
        {
            decodedSamples = MP3DecDecode(pingBuffer);
            if (decodedSamples == 0)
            {
                nextSong();
                return 0;
            }

            // TODO: Aca va filtrado
            // 16bit to 12bit + offset correction
            convert16to12(pingBuffer, pingBuffer, decodedSamples);
            DMA_CH0_PING_config.majorLoopCounts = decodedSamples;
            EDMA_SubmitTransfer(&DMA_CH3_Handle, &DMA_CH0_PING_config);
        }

        state = WAITING_TO_DECODE;
        GPIO_PinWrite(GPIOB, 20, 0);
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

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* DMA channel DMA_CH3 callback function */
void DMA_callback(edma_handle_t *handle, void *data, bool, uint32_t)
{

    if (currBuffPlaying == pingBuffer)
    {
        currBuffPlaying = pongBuffer;
    }
    else
    {
        currBuffPlaying = pingBuffer;
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
    static DIR directory;
    static FILINFO fileInfo;

    FRESULT result = f_opendir(&directory, path); // Open the directory
    if (result != FR_OK)
    {
        printf("Error opening directory: %d\n", result);
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
                printf("MP3 File: %s/%s\n", path, fileInfo.fname);
                // Add the file to the list
                pushtoBack(newSong(path, fileInfo.fname));
            }
        }
    }

    f_closedir(&directory); // Close the directory
}

void convert16to12(const int16_t *input, uint16_t *output, size_t numSamples)
{
    for (size_t i = 0; i < numSamples; ++i)
    {
        // Shift right by 4 bits to discard the least significant 4 bits
        int16_t shiftedValue = input[i] >> 4;

        // Apply an offset to center the 12-bit samples at 2048
        shiftedValue += 2048;

        // Make sure the value is within the 12-bit unsigned range
        if (shiftedValue < 0)
        {
            // For negative values, set output to 0
            output[i] = 0;
        }
        else
        {
            // For non-negative values, ensure the value fits within 12 bits
            output[i] = (uint16_t)(shiftedValue > 0xFFF ? 0xFFF : shiftedValue);
        }
    }
}
