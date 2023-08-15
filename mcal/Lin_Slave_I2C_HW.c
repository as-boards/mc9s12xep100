/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2023 Parai Wang <parai@foxmail.com>
 * ref:
 * http://www.mosaic-industries.com/embedded-systems/sbc-single-board-computers/freescale-hcs12-9s12-c-language/instrument-control/microcontroller-i2c-bus
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include "Mcu.h"
#include "Lin_Slave_I2C_Cfg.h"
#include "Lin_Slave_I2C_Priv.h"
/* ================================ [ MACROS    ] ============================================== */
#define I2C_MODE_UNKNOWN 0
#define I2C_MODE_READ 1
#define I2C_MODE_WRITE 2

//#define LIN_I2C_USE_TRACE_EVENT
#ifdef LIN_I2C_USE_TRACE_EVENT
#define LIN_I2C_PUT_EVENT(evt)                                                                     \
  do {                                                                                             \
    u8I2CISRStatus[u16I2CISRIndex++] = evt;                                                        \
    if (u16I2CISRIndex >= ARRAY_SIZE(u8I2CISRStatus)) {                                            \
      u16I2CISRIndex = 0;                                                                          \
    }                                                                                              \
  } while (0)
#else
#define LIN_I2C_PUT_EVENT(evt)
#endif

#ifdef __AS_BOOTLOADER__
#define ISRNO_VIIC0 VectorNumber_Viic0
#else
#define ISRNO_VIIC0
#endif
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
static boolean bI2C0Busy;

#ifdef LIN_I2C_USE_TRACE_EVENT
static uint8_t u8I2CISRStatus[128];
static uint16_t u16I2CISRIndex = 0;
#endif
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
void Lin_Slave_I2C_HwInit(uint8_t i2cPort, uint8_t devAddr, uint32_t baudrate) {
  if (0 == i2cPort) {
    IIC0_IBCR = 0;      /* disable it */
    IIC0_IBFD = 0x94;   /* when bus clock is 32MHz, set bps to 100K */
    IIC0_IBSR_IBAL = 1; /* clear IBAL flag */
    IIC0_IBCR2 = 0x00;  /* disable general call, 7 bit address*/
    IIC0_IBAD = (devAddr & 0x7F) << 1;
    IIC0_IBCR = 0xC0; /* enable IIC and enable interrupt, with ack */
    bI2C0Busy = FALSE;
  }
}

Std_ReturnType Lin_Slave_I2C_HwRead(uint8_t i2cPort, uint8_t *data) {
  Std_ReturnType ret = E_NOT_OK;
  if (0 == i2cPort) {
    if (0 == (IIC0_IBCR & (1 << 4))) { /* Rx Mode */
      *data = IIC0_IBDR;
      ret = E_OK;
    }
  }
  return ret;
}

Std_ReturnType Lin_Slave_I2C_HwWrite(uint8_t i2cPort, uint8_t data) {
  Std_ReturnType ret = E_NOT_OK;
  if (0 == i2cPort) {
    // if (0 != (IIC0_IBCR & (1 << 4))) { /* Tx Mode */
    /* anyway force to Tx mode */
    IIC0_IBCR |= (1 << 4); /* set Tx Mode */
    IIC0_IBDR = data;
    ret = E_OK;
    //}
  }
  return ret;
}

Std_ReturnType Lin_Slave_I2C_HwEnableAck(uint8_t i2cPort, boolean enable) {
  Std_ReturnType ret = E_NOT_OK;
  if (0 == i2cPort) {
    if (enable) {
      IIC0_IBCR &= ~(1 << 3); /* ack = 0 */
    } else {
      IIC0_IBCR |= (1 << 3); /* ack = 1 */
    }
    ret = E_OK;
  }
  return ret;
}

Std_ReturnType Lin_Slave_I2C_Stop(uint8_t i2cPort) {
  Std_ReturnType ret = E_NOT_OK;
  uint8_t u8Data;
  if (0 == i2cPort) {
    IIC0_IBCR &= ~(1 << 4); /* set Rx Mode */
    IIC0_IBCR &= ~(1 << 3); /* ack = 0 */
    /* a dummy read then releases the SCL line so that the master can generate a STOP signal */
    u8Data = IIC0_IBDR;
    (void)u8Data;
    ret = E_OK;
  }

  return ret;
}

void Lin_Slave_I2C_HwMainFunction(uint8_t i2cPort) {
  uint8_t u8Status;
  if (0 == i2cPort) {
    if (TRUE == bI2C0Busy) {
      IIC0_IBCR &= ~(1 << 6); /* IBIE = 0*/
      u8Status = IIC0_IBSR;
      if (0 == (u8Status & (1 << 5))) { /* IBB = 0 */
        Lin_Slave_I2C_Event(0, LIN_EVENT_STOP);
        bI2C0Busy = FALSE;
        IIC0_IBCR |= (1 << 3); /* ack = 1 */
      }
      IIC0_IBCR |= (1 << 6); /* IBIE = 1*/
    }
  }
}

#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt ISRNO_VIIC0 void Lin_Slave_I2C_ISR(void) {
  uint8_t u8Status = IIC0_IBSR;
  uint8_t u8Data;

  LIN_I2C_PUT_EVENT(u8Status);
  IIC0_IBSR = u8Status; /* clear IBAL and IBIF if set */

  if (u8Status & (1 << 5)) { /* IBB */
    if (FALSE == bI2C0Busy) {
      Lin_Slave_I2C_Event(0, LIN_EVENT_START);
    }
    bI2C0Busy = TRUE;
  }

  if (u8Status & (1 << 4)) { /* IBAL set */
    Lin_Slave_I2C_Event(0, LIN_EVENT_ERROR);
  } else if (u8Status & (1 << 6)) { /* Addressed as a Slave Bit is set */
    IIC0_IBCR &= ~(1 << 3);         /* ack = 0 */
    if (u8Status & (1 << 2)) {      /* SRW is set */
      IIC0_IBCR |= (1 << 4);        /* set Tx Mode */
      Lin_Slave_I2C_Event(0, LIN_EVENT_READ);
    } else {
      IIC0_IBCR &= ~(1 << 4); /* set Rx Mode */
      Lin_Slave_I2C_Event(0, LIN_EVENT_WRITE);
      u8Data = IIC0_IBDR; /* a dummy read */
      (void)u8Data;
    }
  } else if (u8Status & (1 << 7)) { /* TCF */
    Lin_Slave_I2C_Event(0, LIN_EVENT_TRANSFER_COMPLETE);
  }
}

#pragma CODE_SEG DEFAULT
