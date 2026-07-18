/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "MP3Dec.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "sdmmc_config.h"
#include "fsl_sysmpu.h"
#include "MK64F12.h"
#include <mp3dec.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define TAG_SIZE 128

typedef struct
{
    char tag[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    unsigned char genre;
} ID3Tag;
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
/*
Function for reading all the mp3 files in the SD card in all folders and adding them to the song list
*/
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static HMP3Decoder mp3Dec;

static FIL mp3File;

static MP3FrameInfo mp3Info;

static bool fileOpen = false;

static FRESULT error;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool MP3DecInit()
{
    mp3Dec = MP3InitDecoder();

    return mp3Dec == NULL;
}

int MP3SelectSong(song_t *song)
{
    static char mp3path[256] = {0};
    static BYTE readBuff[MAINBUF_SIZE]; // Buffer to store file content
    UINT br;
    int err = 1, offset;

    if (fileOpen)
    {
        f_close(&mp3File);
        fileOpen = false;
    }

    // merge path with filename to get full file directory
    strcpy(mp3path, song->path);
    strcat(mp3path, "/");
    strcat(mp3path, song->filename);

    error = f_open(&mp3File, mp3path, FA_READ);
    fileOpen = error == FR_OK;
    if (!fileOpen)
    {
        return 1;
    }

    while (!f_eof(&mp3File) && err)
    { // Read until Frame valid or EOF
        if (f_read(&mp3File, readBuff, MAINBUF_SIZE, &br) == FR_OK)
        { // Save file content in readBuff

            offset = MP3FindSyncWord(readBuff, br);
            if (offset < 0)
            {
                continue; // Skip block
            }

            err = MP3GetNextFrameInfo(mp3Dec, &mp3Info, readBuff + offset);
            if (err)
            {
                f_lseek(&mp3File, f_tell(&mp3File) - br + offset + 1); // Move file pointer next to last frame sync found
            }
        }
        else
        { // Error reading file
            break;
        }
    }

    if (!err)
    {
        if (f_lseek(&mp3File, f_tell(&mp3File) - br + offset) == FR_OK)
        { // File pointer to Start Of Frame

            return 0;
        }
    }

    f_close(&mp3File);
    fileOpen = false;
    return 1;
}

int MP3DecDecode(int16_t *buffer, uint32_t *samprate)
{
    static BYTE readBuff[MAINBUF_SIZE];                 // Buffer to store file content
    static int16_t stereoOutBuff[DEC_BUFFER_SIZE * 2]; // Buffer to store stereo samples
    UINT bytesRead;
    int offset;
    int err = 1;
    bool needSync = true;
    UINT bytesLeft = 0;

    if (!fileOpen)
    {
        return 0;
    }

    while (!f_eof(&mp3File))
    {

        // Read file content in readBuff
        if (f_read(&mp3File, readBuff + bytesLeft, MAINBUF_SIZE - bytesLeft, &bytesRead) != FR_OK)
        {
            return 0;
        }

        bytesRead += bytesLeft; // Update total bytes read
        bytesLeft = 0;          // Reset bytes left

        BYTE *readBuffPtr = readBuff;

        // Sync to Header
        if (needSync)
        {
            offset = MP3FindSyncWord(readBuffPtr, bytesRead);
            if (offset < 0)
            {
                needSync = true; // Sync not found in the buffer
                continue;        // Skip block
            }

            needSync = false;               // Sync found
            bytesLeft = bytesRead - offset; // Update bytes left to read
            readBuffPtr += offset;          // Move buffer pointer to start of frame
        }

                // Decode
        err = MP3GetNextFrameInfo(mp3Dec, &mp3Info, readBuffPtr);
        *samprate = mp3Info.samprate;

        if (!err)
        {
            if (mp3Info.nChans == 1) // File is mono
            {
                err = MP3Decode(mp3Dec, &readBuffPtr, &bytesRead, buffer, 0);
                bytesLeft = bytesRead;
            }
            else if (mp3Info.nChans == 2) // File is stereo
            {
                err = MP3Decode(mp3Dec, &readBuffPtr, &bytesRead, stereoOutBuff, 0);
                bytesLeft = bytesRead;

                for (int i = 0; i < DEC_BUFFER_SIZE; i++)
                {
                    buffer[i] = ((int32_t)stereoOutBuff[i * 2] + (int32_t)stereoOutBuff[i * 2 + 1]) / 2;
                }
            }

            f_lseek(&mp3File, f_tell(&mp3File) - bytesLeft);

            return (mp3Info.nChans == 1) ? mp3Info.outputSamps : mp3Info.outputSamps / 2;
        }

        switch (err)
        {
        case ERR_MP3_INDATA_UNDERFLOW:
        case ERR_MP3_MAINDATA_UNDERFLOW:
            if (!f_eof(&mp3File))
            {
                // Move file pointer to start of last frame
                f_lseek(&mp3File, f_tell(&mp3File) - bytesLeft);
                bytesLeft = 0;
            }
            break;
        case ERR_MP3_INVALID_FRAMEHEADER:
        case ERR_MP3_INVALID_HUFFCODES:
            // Move file pointer next to start of last frame and resync
            needSync = true;
            f_lseek(&mp3File, f_tell(&mp3File) - bytesRead + offset);
            bytesLeft = 0;
            break;
        default:
            // Move file pointer next to start of last frame and resync
            needSync = true;
            f_lseek(&mp3File, f_tell(&mp3File) - bytesRead + offset);
            bytesLeft = 0;
            break;
        }
    }

    return 0;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
