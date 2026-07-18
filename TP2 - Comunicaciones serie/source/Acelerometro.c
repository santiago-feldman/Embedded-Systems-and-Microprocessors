/***************************************************************************/ /**
   @file     +Nombre del archivo (ej: template.c)+
   @brief    +Descripcion del archivo+
   @author   +Nombre del autor (ej: Salvador Allende)+
  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Acelerometro.h"
#include "I2C.h"
#include "timer.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR 0x1D // with pins SA0=0, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS 0x00
#define FXOS8700CQ_WHOAMI 0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 0x2A
#define FXOS8700CQ_M_CTRL_REG1 0x5B
#define FXOS8700CQ_M_CTRL_REG2 0x5C
#define FXOS8700CQ_WHOAMI_VAL 0xC7

// number of bytes to be read from the FXOS8700CQ
#define FXOS8700CQ_READ_LEN 12 // 6 channels = 12 bytes

// Sampling time
#define SAMPLING_TIME 25
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void readAccel_CB();
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static SRAWDATA magData = {0};
static SRAWDATA accData = {0};
static uint32_t cbTimerId = 0;
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
// function configures FXOS8700CQ combination accelerometer and magnetometer sensor
uint32_t initAccelerometer()
{
  uint8_t databyte;
  I2CInit(0, 0x11, 0);

  // read and check the FXOS8700CQ WHOAMI register
  if (i2cRequestSlaveRegData(0, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_WHOAMI, 1))
  {
    while (!i2cIsRxMsg(0))
      ;
    i2cReadMsg(0, &databyte, 1);
  }
  if (databyte != FXOS8700CQ_WHOAMI_VAL)
  {
    return (-1);
  }
  // write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into
  // standby
  // [7-1] = 0000 000
  // [0]: active=0
  databyte = 0x00;
  if (i2cWriteSlaveReg(0, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_CTRL_REG1, &databyte, 1) == 0)
  {
    return (-1);
  }
  while (isI2CBusBusy(0) || !i2cIsTxMsgComplete(0))
    ;
  // write 0001 1111 = 0x1F to magnetometer control register 1
  // [7]: m_acal=0: auto calibration disabled
  // [6]: m_rst=0: no one-shot magnetic reset
  // [5]: m_ost=0: no one-shot magnetic measurement
  // [4-2]: m_os=111=7: 8x oversampling (for 200Hz) to reduce magnetometer noise
  // [1-0]: m_hms=11=3: select hybrid mode with accel and magnetometer active
  databyte = 0x1F;
  if (i2cWriteSlaveReg(0, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_M_CTRL_REG1, &databyte, 1) == 0)
  {
    return (-1);
  }
  while (isI2CBusBusy(0) || !i2cIsTxMsgComplete(0))
    ;
  // write 0010 0000 = 0x20 to magnetometer control register 2
  // [7]: reserved
  // [6]: reserved
  // [5]: hyb_autoinc_mode=1 to map the magnetometer registers to follow the
  // accelerometer registers
  // [4]: m_maxmin_dis=0 to retain default min/max latching even though not used
  // [3]: m_maxmin_dis_ths=0
  // [2]: m_maxmin_rst=0
  // [1-0]: m_rst_cnt=00 to enable magnetic reset each cycle
  databyte = 0x20;
  if (i2cWriteSlaveReg(0, FXOS8700CQ_SLAVE_ADDR,
                       FXOS8700CQ_M_CTRL_REG2,
                       &databyte, 1) == 0)
  {
    return (-1);
  }
  while (isI2CBusBusy(0) || !i2cIsTxMsgComplete(0))
    ;
  // write 0000 0001= 0x01 to XYZ_DATA_CFG register
  // [7]: reserved
  // [6]: reserved
  // [5]: reserved
  // [4]: hpf_out=0
  // [3]: reserved
  // [2]: reserved
  // [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg / LSB
  databyte = 0x01;
  if (i2cWriteSlaveReg(0, FXOS8700CQ_SLAVE_ADDR,
                       FXOS8700CQ_XYZ_DATA_CFG,
                       &databyte, 1) == 0)
  {
    return (-1);
  }
  while (isI2CBusBusy(0) || !i2cIsTxMsgComplete(0))
    ;
  // write 0000 1101 = 0x0D to accelerometer control register 1
  // [7-6]: aslp_rate=00
  // [5-3]: dr=001 for 200Hz data rate (when in hybrid mode)
  // [2]: lnoise=1 for low noise mode
  // [1]: f_read=0 for normal 16 bit reads
  // [0]: active=1 to take the part out of standby and enable sampling
  databyte = 0x0D;
  if (i2cWriteSlaveReg(0, FXOS8700CQ_SLAVE_ADDR,
                       FXOS8700CQ_CTRL_REG1, &databyte,
                       1) == 0)
  {
    return (-1);
  }
  while (isI2CBusBusy(0) || !i2cIsTxMsgComplete(0))
    ;
  cbTimerId = createTimer(SAMPLING_TIME, &readAccel_CB, PERIODIC);
  startTimer(cbTimerId);
  // normal return
  return (0);
}

// read status and the three channels of accelerometer and magnetometer data from
// FXOS8700CQ (13 bytes)
void ReadAccelMagnData(SRAWDATA *pAccelData, SRAWDATA *pMagnData)
{
  *pAccelData = accData;
  *pMagnData = magData;
}
/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static void readAccel_CB()
{
  static uint8_t Buffer[FXOS8700CQ_READ_LEN]; // read buffer
  // read FXOS8700CQ_READ_LEN=13 bytes (status byte and the six channels of data)

  i2cReadMsg(0, Buffer, FXOS8700CQ_READ_LEN);

  // copy the 14 bit accelerometer byte data into 16 bit words
  accData.x = (int16_t)(((Buffer[0] << 8) | Buffer[1])) >> 2;
  accData.y = (int16_t)(((Buffer[2] << 8) | Buffer[3])) >> 2;
  accData.z = (int16_t)(((Buffer[4] << 8) | Buffer[5])) >> 2;
  // copy the magnetometer byte data into 16 bit words
  magData.x = (Buffer[6] << 8) | Buffer[7];
  magData.y = (Buffer[8] << 8) | Buffer[9];
  magData.z = (Buffer[10] << 8) | Buffer[14];

  i2cRequestSlaveRegData(0, FXOS8700CQ_SLAVE_ADDR, FXOS8700CQ_STATUS + 1, FXOS8700CQ_READ_LEN);
  // normal return
}
