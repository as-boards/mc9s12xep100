/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2023 Parai Wang <parai@foxmail.com>
 *
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include "Lin_Slave_I2C_Cfg.h"
#include "Lin_Slave_I2C_Priv.h"
#include "LinTp.h"
#include "Dcm.h"
#include "Dcm_Internal.h"
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
  20000,  /* 20 ms as tick base is 10ms */
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
  PduInfoType pdu;

  if (LIN_E_SLAVE_RECEIVED_OK == notifyResult) {
    if ((PduInfo->Dl == 8) && (0xD0 == PduInfo->Pid)) {
      pdu.SduDataPtr = PduInfo->SduPtr;
      pdu.SduLength = PduInfo->Dl;
      LinTp_RxIndication((PduIdType)channel, &pdu);
      ret = E_OK;
    }
  }

  return ret;
}

static Std_ReturnType Lin_Slave_DiagRespose(uint8_t channel, Lin_PduType *PduInfo,
                                            Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;
  PduInfoType pdu;

  if (LIN_E_SLAVE_TRIGGER_TRANSMIT == notifyResult) {
    if ((PduInfo->Dl == 8) && (0xD1 == PduInfo->Pid)) {
      pdu.SduDataPtr = PduInfo->SduPtr;
      pdu.SduLength = PduInfo->Dl;
      ret = LinTp_TriggerTransmit((PduIdType)channel, &pdu);
    }
  } else if (LIN_E_SLAVE_TRANSMIT_OK == notifyResult) {
    LinTp_TxConfirmation((PduIdType)channel, E_OK);
    ret = E_OK;
  }
  return ret;
}

static Std_ReturnType Lin_Slave_ReadAnswerPatten(uint8_t channel, Lin_PduType *PduInfo,
                                                 Std_ReturnType notifyResult) {
  Std_ReturnType ret = E_NOT_OK;
  Dcm_ContextType *context = Dcm_GetContext();

  if (LIN_E_SLAVE_TRIGGER_TRANSMIT == notifyResult) {
    if ((PduInfo->Dl == 1) && (0xE0 == PduInfo->Pid)) {
      PduInfo->SduPtr[0] = 0;
      if (DCM_BUFFER_IDLE == context->rxBufferState) {
        PduInfo->SduPtr[0] |= 1 << 7; /* ECU is ready to process new diag request */
      }
      if (DCM_BUFFER_IDLE != context->txBufferState) {
        PduInfo->SduPtr[0] |= 1 << 0; /* D2 register shall be read out */
      }
      ret = E_OK;
    }
  } else if (LIN_E_SLAVE_TRANSMIT_OK == notifyResult) {
    ret = E_OK;
  }
  return ret;
}
/* ================================ [ FUNCTIONS ] ============================================== */
