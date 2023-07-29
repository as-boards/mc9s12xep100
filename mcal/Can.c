/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2019-2023 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include "Mcu.h"
#include "Can.h"
/* ================================ [ MACROS    ] ============================================== */
/* this just alow only one HTH/HRH for each CAN controller */
#define CAN_MAX_HOH 1
/* ================================ [ TYPES     ] ============================================== */
struct can_msg {
  unsigned int id;
  boolean RTR;
  unsigned char data[8];
  unsigned char len;
  unsigned char prty;
};
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
static uint16_t lOpenFlag = 0;
static uint16_t lWriteFlag = 0;
static PduIdType lswPduHandle[CAN_MAX_HOH];
/* ================================ [ LOCALS    ] ============================================== */
static void INIT_CAN0(void) {
  if (CAN0CTL0_INITRQ == 0) // 查询是否进入初始化状态
    CAN0CTL0_INITRQ = 1;    // 进入初始化状态

  while (CAN0CTL1_INITAK == 0)
    ; //等待进入初始化状态

  CAN0BTR0_SJW = 0; //设置同步
  CAN0BTR0_BRP = 7; //设置波特率
  CAN0BTR1 = 0x1c;  //设置时段1和时段2的Tq个数 ,总线频率为250kb/s

  // 关闭滤波器
  CAN0IDMR0 = 0xFF;
  CAN0IDMR1 = 0xFF;
  CAN0IDMR2 = 0xFF;
  CAN0IDMR3 = 0xFF;
  CAN0IDMR4 = 0xFF;
  CAN0IDMR5 = 0xFF;
  CAN0IDMR6 = 0xFF;
  CAN0IDMR7 = 0xFF;

  CAN0CTL1 = 0xC0; //使能MSCAN模块,设置为一般运行模式、使用总线时钟源

  CAN0CTL0 = 0x00; //返回一般模式运行

  while (CAN0CTL1_INITAK)
    ; //等待回到一般运行模式

  while (CAN0CTL0_SYNCH == 0)
    ; //等待总线时钟同步

  CAN0RIER_RXFIE = 0; //禁止接收中断
}

static boolean MSCAN0SendMsg(struct can_msg *msg) {
  unsigned char send_buf, sp;

  // 检查数据长度
  if (msg->len > 8)
    return (FALSE);

  // 检查总线时钟
  if (CAN0CTL0_SYNCH == 0)
    return (FALSE);

  send_buf = 0;
  do {
    // 寻找空闲的缓冲器
    CAN0TBSEL = CAN0TFLG;
    send_buf = CAN0TBSEL;
  } while (!send_buf);

  // 写入标识符
  CAN0TXIDR0 = (unsigned char)(msg->id >> 3);
  CAN0TXIDR1 = (unsigned char)(msg->id << 5);

  if (msg->RTR)
    // RTR = 阴性
    CAN0TXIDR1 |= 0x10;

  // 写入数据
  for (sp = 0; sp < msg->len; sp++)
    *((&CAN0TXDSR0) + sp) = msg->data[sp];

  // 写入数据长度
  CAN0TXDLR = msg->len;

  // 写入优先级
  CAN0TXTBPR = msg->prty;

  // 清 TXx 标志 (缓冲器准备发送)
  CAN0TFLG = send_buf;

  return (TRUE);
}

static boolean MSCAN0GetMsg(struct can_msg *msg) {
  unsigned char sp2;

  // 检测接收标志
  if (!(CAN0RFLG_RXF))
    return (FALSE);

  // 检测 CAN协议报文模式 （一般/扩展） 标识符
  if (CAN0RXIDR1_IDE)
    // IDE = Recessive (Extended Mode)
    return (FALSE);

  // 读标识符
  msg->id = (unsigned int)(CAN0RXIDR0 << 3) | (unsigned char)(CAN0RXIDR1 >> 5);

  if (CAN0RXIDR1 & 0x10)
    msg->RTR = TRUE;
  else
    msg->RTR = FALSE;

  // 读取数据长度
  msg->len = CAN0RXDLR;

  // 读取数据
  for (sp2 = 0; sp2 < msg->len; sp2++)
    msg->data[sp2] = *((&CAN0RXDSR0) + sp2);

  // 清 RXF 标志位 (缓冲器准备接收)
  CAN0RFLG = 0x01;

  return (TRUE);
}

/* ================================ [ FUNCTIONS ] ============================================== */
void Can_Init(const Can_ConfigType *Config) {
  lOpenFlag = 0;
  lWriteFlag = 0;
}

Std_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ControllerStateType Transition) {
  Std_ReturnType ret = E_NOT_OK;

  switch (Transition) {
  case CAN_CS_STARTED:
    if (0 == Controller) {
      INIT_CAN0();
      lOpenFlag |= (1 << Controller);
      lWriteFlag = 0;
      ret = E_OK;
    }
    break;
  case CAN_CS_STOPPED:
  case CAN_CS_SLEEP:
    break;
  default:
    break;
  }

  return ret;
}

Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType *PduInfo) {
  Std_ReturnType ret = E_OK;
  boolean r;
  struct can_msg msg;
  if (lOpenFlag & (1 << Hth)) {
    if (0 == (lWriteFlag & (1 << Hth))) {
      if (0 == Hth) {
        msg.id = PduInfo->id;
        memcpy(msg.data, PduInfo->sdu, PduInfo->length);
        msg.len = PduInfo->length;
        msg.RTR = FALSE;
        msg.prty = 0;
        r = MSCAN0SendMsg(&msg);
        if (TRUE == r) {
          lWriteFlag |= (1 << Hth);
          lswPduHandle[Hth] = PduInfo->swPduHandle;
        }
      } else {
        ret = E_NOT_OK;
      }
    } else {
      ret = CAN_BUSY;
    }
  } else {
    ret = E_NOT_OK;
  }
}

void Can_DeInit(void) {
}

void Can_MainFunction_Write(void) {
  PduIdType swPduHandle;
  if (lOpenFlag & 0x01) {
    if (lWriteFlag & 0x01) {
      swPduHandle = lswPduHandle[0];
      lWriteFlag &= ~(1 << 0);
      CanIf_TxConfirmation(swPduHandle);
    }
  }
}

void Can_MainFunction_Read(void) {
  boolean r;
  struct can_msg msg;
  Can_HwType Mailbox;
  PduInfoType PduInfo;
  if (lOpenFlag & 0x01) {
    r = MSCAN0GetMsg(&msg);
    if (r) {
      Mailbox.CanId = msg.id;
      Mailbox.ControllerId = 0;
      Mailbox.Hoh = 0;
      PduInfo.SduLength = msg.len;
      PduInfo.SduDataPtr = msg.data;
      PduInfo.MetaDataPtr = (uint8_t *)&Mailbox;
      CanIf_RxIndication(&Mailbox, &PduInfo);
    }
  }
}

void Can_0_RxIsr(void) {
}

void Can_0_TxIsr(void) {
}

void Can_0_ErrIsr(void) {
}

void Can_0_WakeIsr(void) {
}