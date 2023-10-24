/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2023 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include "Mcu.h"
#include "Lin.h"
#include "LinIf.h"
#include <string.h>
#include "Std_Timer.h"
/* ================================ [ MACROS    ] ============================================== */
#ifdef __AS_BOOTLOADER__
#define ISRNO_VSPI0 VectorNumber_Vspi0
#else
#define ISRNO_VSPI0
#endif

#define LIN_SPI_USE_TRACE_EVENT
#ifdef LIN_SPI_USE_TRACE_EVENT
#define LIN_SPI_PUT_EVENT(evt)                                                                     \
  do {                                                                                             \
    u8SpiEvents[u16SpiIndex++] = evt;                                                              \
    if (u16SpiIndex >= ARRAY_SIZE(u8SpiEvents)) {                                                  \
      u16SpiIndex = 0;                                                                             \
    }                                                                                              \
  } while (0)
#else
#define LIN_SPI_PUT_EVENT(evt)
#endif

#define LIN_SPI_IDEL 0
#define LIN_SPI_ID 1
#define LIN_SPI_DLC 2
#define LIN_SPI_RX_DATA 3
#define LIN_SPI_RX_CHECKSUM 4
#define LIN_SPI_TX_DATA 5
#define LIN_SPI_TX_CHECKSUM 6
#define LIN_SPI_TX_CHECKSUM_DONE 7
#define LIN_SPI_IGNORE 8

#ifndef LIN_MAX_DATA_SIZE
#define LIN_MAX_DATA_SIZE 64
#endif
/* ================================ [ TYPES     ] ============================================== */
typedef struct {
  uint8_t state;
  uint16_t id;
  uint8_t dlc;
  uint8_t data[LIN_MAX_DATA_SIZE];
  uint8_t checksum;
  uint8_t index;
  Std_TimerType timer;
} Lin_SpiContextType;
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
#ifdef LIN_SPI_USE_TRACE_EVENT
static uint8_t u8SpiEvents[128];
static uint16_t u16SpiIndex = 0;
#endif

static Lin_SpiContextType lSpiContext;
/* ================================ [ LOCALS    ] ============================================== */
static uint8_t Lin_SpiChecksum(Lin_SpiContextType *context) {
  uint8_t checksum = 0;
  int i;
  checksum += (lSpiContext.id >> 8) & 0xFF;
  checksum += lSpiContext.id & 0xFF;
  checksum += lSpiContext.dlc;
  for (i = 0; i < lSpiContext.dlc; i++) {
    checksum += lSpiContext.data[i];
  }
  return ~checksum;
}
/* ================================ [ FUNCTIONS ] ============================================== */
void Lin_Init(const Lin_ConfigType *ConfigPtr) {
}

Std_ReturnType Lin_SetControllerMode(uint8_t Channel, Lin_ControllerStateType Transition) {
  if (LIN_CS_STARTED == Transition) {
    memset(&lSpiContext, 0, sizeof(lSpiContext));
    Std_TimerStop(&lSpiContext.timer);
    SPI0CR2 = 0x00;
    SPI0CR1 = 0b11100100; /* SPI enable and slave mode, CPOL=0 CPHA=1 */
  } else {
    SPI0CR1 = 0;
  }
}

Std_ReturnType Lin_SendFrame(uint8_t Channel, const Lin_PduType *PduInfoPtr) {
}

Lin_StatusType Lin_GetStatus(uint8_t Channel, uint8_t **Lin_SduPtr) {
}

void Lin_MainFunction_Read(void) {
}

void Lin_MainFunction(void) {
  std_time_t elapsed;
  if (Std_IsTimerStarted(&lSpiContext.timer)) {
    elapsed = Std_GetTimerElapsedTime(&lSpiContext.timer);
    if (elapsed > 50000) {
      lSpiContext.state = LIN_SPI_IDEL;
    }
  }
}

#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt ISRNO_VSPI0 void Lin_Slave_SPI_ISR(void) {
  uint8_t u8Status = SPI0SR;
  uint8_t u8Data;
  Std_ReturnType ret;
  Lin_PduType pdu;

  if (u8Status & (1 << 7)) { /* SPIF */
    u8Data = SPI0DRL;
    switch (lSpiContext.state) {
    case LIN_SPI_IDEL:
      SPI0DRL = 0xF1;
      LIN_SPI_PUT_EVENT(u8Data);
      lSpiContext.id = u8Data << 8;
      lSpiContext.state = LIN_SPI_ID;
      Std_TimerStart(&lSpiContext.timer);
      break;
    case LIN_SPI_ID:
      SPI0DRL = 0xF2;
      LIN_SPI_PUT_EVENT(u8Data);
      lSpiContext.id |= u8Data;
      lSpiContext.state = LIN_SPI_DLC;
      break;
    case LIN_SPI_DLC:
      LIN_SPI_PUT_EVENT(u8Data);
      lSpiContext.dlc = u8Data;
      lSpiContext.index = 0;
      pdu.Pid = lSpiContext.id;
      ret = LinIf_HeaderIndication(0, &pdu);
      if (E_OK == ret) {
        if (lSpiContext.dlc != pdu.Dl) {
          ret = E_NOT_OK;
        }
      }
      if (E_OK == ret) {
        if (LIN_FRAMERESPONSE_RX == pdu.Drc) {
          SPI0DRL = 0xF3;
          lSpiContext.state = LIN_SPI_RX_DATA;
        } else if (LIN_FRAMERESPONSE_TX == pdu.Drc) {
          SPI0DRL = pdu.SduPtr[0];
          LIN_SPI_PUT_EVENT(pdu.SduPtr[0]);
          lSpiContext.state = LIN_SPI_TX_DATA;
          memcpy(lSpiContext.data, pdu.SduPtr, lSpiContext.dlc);
          lSpiContext.checksum = Lin_SpiChecksum(&lSpiContext);
        } else {
          SPI0DRL = 0xF4;
          lSpiContext.state = LIN_SPI_IGNORE;
        }
      } else {
        SPI0DRL = 0xF5;
        lSpiContext.state = LIN_SPI_IGNORE;
      }
      break;
    case LIN_SPI_RX_DATA:
      SPI0DRL = 0xF6;
      LIN_SPI_PUT_EVENT(u8Data);
      lSpiContext.data[lSpiContext.index] = u8Data;
      lSpiContext.index++;
      if (lSpiContext.index >= lSpiContext.dlc) {
        lSpiContext.state = LIN_SPI_RX_CHECKSUM;
      }
      break;
    case LIN_SPI_RX_CHECKSUM:
      SPI0DRL = 0xF7;
      LIN_SPI_PUT_EVENT(u8Data);
      lSpiContext.state = LIN_SPI_IDEL;
      lSpiContext.checksum = Lin_SpiChecksum(&lSpiContext);
      if (u8Data == lSpiContext.checksum) {
        LinIf_RxIndication(0, lSpiContext.data);
      }
      Std_TimerStop(&lSpiContext.timer);
      break;
    case LIN_SPI_TX_DATA:
      lSpiContext.index++;
      if (lSpiContext.index >= lSpiContext.dlc) {
        SPI0DRL = lSpiContext.checksum;
        LIN_SPI_PUT_EVENT(lSpiContext.checksum);
        lSpiContext.state = LIN_SPI_TX_CHECKSUM;
      } else {
        SPI0DRL = lSpiContext.data[lSpiContext.index];
        LIN_SPI_PUT_EVENT(lSpiContext.data[lSpiContext.index]);
      }
      break;
    case LIN_SPI_TX_CHECKSUM:
      SPI0DRL = 0xF8;
      lSpiContext.state = LIN_SPI_TX_CHECKSUM_DONE;
      break;
    case LIN_SPI_TX_CHECKSUM_DONE:
      SPI0DRL = 0xF9;
      lSpiContext.state = LIN_SPI_IDEL;
      Std_TimerStop(&lSpiContext.timer);
      break;
    case LIN_SPI_IGNORE:
      SPI0DRL = 0xFA;
      lSpiContext.index++;
      if (lSpiContext.index >= lSpiContext.dlc) {
        lSpiContext.state = LIN_SPI_IDEL;
      }
      break;
    default:
      SPI0DRL = 0xFE;
      LIN_SPI_PUT_EVENT(0xFE);
      break;
    }
  } else if (u8Status & (1 << 5)) { /* SPTEF */
    SPI0DRL = 0xFB;
    LIN_SPI_PUT_EVENT(0xFB);
  } else {
  }
}
#pragma CODE_SEG DEFAULT
