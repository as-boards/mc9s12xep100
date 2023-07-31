/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2023 Parai Wang <parai@foxmail.com>
 *
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include "Lin_Slave_I2C_Cfg.h"
#include "Lin_Slave_I2C_Priv.h"
/* ================================ [ MACROS    ] ============================================== */
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
static Std_ReturnType Lin_Slave_DiagRequest(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult);
static Std_ReturnType Lin_Slave_DiagRespose(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult);
static Std_ReturnType Lin_Slave_ReadAnswerPatten(uint8_t channel, Lin_PduType *PduInfo,
                                                 Std_ReturnType notifyResult);
/* ================================ [ DATAS     ] ============================================== */
static const Lin_FrameDefType Lin_FrameDefs[] = {
  {Lin_Slave_DiagRequest, 0xD0, LIN_FRAMERESPONSE_RX, 8},
  {Lin_Slave_DiagRespose, 0xD1, LIN_FRAMERESPONSE_TX, 8},
  {Lin_Slave_ReadAnswerPatten, 0xE0, LIN_FRAMERESPONSE_TX, 1},
};

static const Lin_ChannelConfigType Lin_ChannelConfigs[] = {
  Lin_FrameDefs,
  ARRAY_SIZE(Lin_FrameDefs),
  100000, /* 100 kbps */
  100000, /* 10 ms */
  0x51,   /* devAddr */
  0,      /* i2cPort */
};

static Lin_ChannelContextType Lin_ChannelContexts[1];

const Lin_ConfigType Lin_I2C_Slave_Config = {
  Lin_ChannelConfigs,
  Lin_ChannelContexts,
  ARRAY_SIZE(Lin_ChannelConfigs),
};
/* ================================ [ LOCALS    ] ============================================== */
static Std_ReturnType Lin_Slave_DiagRequest(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;

  if (LIN_E_SLAVE_RECEIVED_OK == notifyResult) {
    if ((PduInfo->Dl == 8) && (0xD0 == PduInfo->Pid)) {
      ret = E_OK;
    }
  }

  return ret;
}

static Std_ReturnType Lin_Slave_DiagRespose(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;
  const uint8_t response[8] = {0x03, 0x7F, 0x10, 0x31, 0x55, 0x55, 0x55, 0x55};
  if (LIN_E_SLAVE_TRIGGER_TRANSMIT == notifyResult) {
    if ((PduInfo->Dl == 8) && (0xD1 == PduInfo->Pid)) {
      memcpy(PduInfo->SduPtr, response, 8);
      ret = E_OK;
    }
  } else if (LIN_E_SLAVE_TRANSMIT_OK == notifyResult) {
    if ((PduInfo->Dl == 8) && (0xD1 == PduInfo->Pid)) {
      ret = E_OK;
    }
  }
  return ret;
}

static Std_ReturnType Lin_Slave_ReadAnswerPatten(uint8_t channel, Lin_PduType *PduInfo,
                                                 Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;

  if (LIN_E_SLAVE_TRIGGER_TRANSMIT == notifyResult) {
    if ((PduInfo->Dl == 1) && (0xE0 == PduInfo->Pid)) {
      PduInfo->SduPtr[0] = 0xFF;
      ret = E_OK;
    }
  } else if (LIN_E_SLAVE_TRANSMIT_OK == notifyResult) {
    if ((PduInfo->Dl == 1) && (0xE0 == PduInfo->Pid)) {
      ret = E_OK;
    }
  }
  return ret;
}
/* ================================ [ FUNCTIONS ] ============================================== */
