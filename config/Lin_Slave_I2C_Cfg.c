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
  10000,  /* 10 ms */
  0x51,   /* devAddr */
  0,      /* i2cPort */
};

static const Lin_ChannelContextType Lin_ChannelContexts[1];

const Lin_ConfigType Lin_I2C_Slave_Config = {
  Lin_ChannelConfigs,
  Lin_ChannelContexts,
  ARRAY_SIZE(Lin_ChannelConfigs),
};
/* ================================ [ LOCALS    ] ============================================== */
static Std_ReturnType Lin_Slave_DiagRequest(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;

  return ret;
}

static Std_ReturnType Lin_Slave_DiagRespose(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;

  return ret;
}

static Std_ReturnType Lin_Slave_ReadAnswerPatten(uint8_t channel, Lin_PduType *PduInfo,
                                                 Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;

  return ret;
}
/* ================================ [ FUNCTIONS ] ============================================== */
