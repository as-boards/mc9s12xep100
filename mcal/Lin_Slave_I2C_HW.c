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
#include "Lin_Slave.h"
/* ================================ [ MACROS    ] ============================================== */
#define I2C_MODE_UNKNOWN 0
#define I2C_MODE_READ 1
#define I2C_MODE_WRITE 2

/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
static boolean bI2C0Busy;
static uint8_t u8I2C0Mode;
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
void Lin_Slave_I2C_HwInit(uint8_t i2cPort, uint8_t devAddr, uint32_t baudrate) {
  if (0 == i2cPort) {
    IIC0_IBFD = 0x94;   /* when bus clock is 32MHz, set bps to 100K */
    IIC0_IBCR = 0xC8;   /* enable IIC and enable interrupt, no ack */
    IIC0_IBSR_IBAL = 1; /* clear IBAL flag */
    IIC0_IBCR2 = 0x00;  /* disable general call, 7 bit address*/
    IIC0_IBAD = (devAddr & 0x7F) << 1;
    bI2C0Busy = FALSE;
    u8I2C0Mode = I2C_MODE_UNKNOWN;
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
    if (0 != (IIC0_IBCR & (1 << 4))) { /* Tx Mode */
      IIC0_IBDR = data;
      ret = E_OK;
    }
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

void Lin_Slave_I2C_HwMainFunction(uint8_t i2cPort) {
  uint8_t u8Status;
  if (0 == i2cPort) {
    IIC0_IBCR &= ~(1 << 6); /* IBIE = 0*/
    u8Status = IIC0_IBSR;
    if (u8Status & (1 << 5)) { /* IBB */
      if (FALSE == bI2C0Busy) {
        Lin_Slave_I2C_Event(0, LIN_EVENT_START);
      }
      bI2C0Busy = TRUE;
    } else {
      if (TRUE == bI2C0Busy) {
        Lin_Slave_I2C_Event(0, LIN_EVENT_STOP);
      }
      u8I2C0Mode = I2C_MODE_UNKNOWN;
      bI2C0Busy = FALSE;
      IIC0_IBCR |= (1 << 3); /* ack = 1 */
    }
    IIC0_IBCR |= (1 << 6); /* IBIE = 1*/
  }
}

#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt VectorNumber_Viic0 void Lin_Slave_I2C_ISR(void) {
  uint8_t u8Status = IIC0_IBSR;
  uint8_t u8Data;

  IIC0_IBSR = u8Status; /* clear IBAL and IBIF if set */

  if (u8Status & (1 << 5)) { /* IBB */
    if (FALSE == bI2C0Busy) {
      Lin_Slave_I2C_Event(0, LIN_EVENT_START);
    }
    bI2C0Busy = TRUE;
  } else {
    if (TRUE == bI2C0Busy) {
      Lin_Slave_I2C_Event(0, LIN_EVENT_STOP);
    }
    u8I2C0Mode = I2C_MODE_UNKNOWN;
    bI2C0Busy = FALSE;
    IIC0_IBCR |= (1 << 3); /* ack = 1 */
  }

  if (u8Status & (1 << 4)) { /* IBAL set */
    Lin_Slave_I2C_Event(0, LIN_EVENT_ERROR);
  }

  if (u8Status & (1 << 7)) { /* TCF */
    if (I2C_MODE_UNKNOWN != u8I2C0Mode) {
      Lin_Slave_I2C_Event(0, LIN_EVENT_TRANSFER_COMPLETE);
    }

    if (I2C_MODE_WRITE == u8I2C0Mode) {
      if (0 == (u8Status & (1 << 0))) { /* RXAK set */
        Lin_Slave_I2C_Event(0, LIN_EVENT_ACK);
      }
    }
  }

  if (u8Status & (1 << 6)) {   /* Addressed as a Slave Bit is set */
    if (u8Status & (1 << 2)) { /* SRW is set */
      IIC0_IBCR |= (1 << 4);   /* set Tx Mode */
      if (I2C_MODE_READ != u8I2C0Mode) {
        Lin_Slave_I2C_Event(0, LIN_EVENT_READ);
      }
      u8I2C0Mode = I2C_MODE_READ;
    } else {
      IIC0_IBCR &= ~(1 << 4); /* set Rx Mode */
      if (I2C_MODE_WRITE != u8I2C0Mode) {
        Lin_Slave_I2C_Event(0, LIN_EVENT_WRITE);
        u8Data = IIC0_IBDR; /* a dummy read */
        (void)u8Data;
      }
      u8I2C0Mode = I2C_MODE_WRITE;
    }
  }
}

#pragma CODE_SEG DEFAULT
