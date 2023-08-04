/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2021 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include "Std_Types.h"
/* ================================ [ MACROS    ] ============================================== */
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
extern interrupt void Can_0_TxIsr_Entry(void);
extern interrupt void Can_0_RxIsr_Entry(void);
extern interrupt void Can_0_ErrIsr_Entry(void);
extern interrupt void Can_0_WakeIsr_Entry(void);
extern interrupt void Lin_Slave_I2C_ISR(void);
extern interrupt void Isr_SystemTick(void);
extern interrupt void Isr_SoftwareInterrupt(void);
extern interrupt void isr_default(void);
/* ================================ [ DATAS     ] ============================================== */
#pragma DATA_SEG __NEAR_SEG.vectors
const uint16_t tisr_pc[] = {
  /* 119 */ (uint16_t)isr_default,
  /* 118 */ (uint16_t)isr_default,
  /* 117 */ (uint16_t)isr_default,
  /* 116 */ (uint16_t)isr_default,
  /* 115 */ (uint16_t)isr_default,
  /* 114 */ (uint16_t)isr_default,
  /* 113 */ (uint16_t)isr_default,
  /* 112 */ (uint16_t)isr_default,
  /* 111 */ (uint16_t)isr_default,
  /* 110 */ (uint16_t)isr_default,
  /* 109 */ (uint16_t)isr_default,
  /* 108 */ (uint16_t)isr_default,
  /* 107 */ (uint16_t)isr_default,
  /* 106 */ (uint16_t)isr_default,
  /* 105 */ (uint16_t)isr_default,
  /* 104 */ (uint16_t)isr_default,
  /* 103 */ (uint16_t)isr_default,
  /* 102 */ (uint16_t)isr_default,
  /* 101 */ (uint16_t)isr_default,
  /* 100 */ (uint16_t)isr_default,
  /* 99 */ (uint16_t)isr_default,
  /* 98 */ (uint16_t)isr_default,
  /* 97 */ (uint16_t)isr_default,
  /* 96 */ (uint16_t)isr_default,
  /* 95 */ (uint16_t)isr_default,
  /* 94 */ (uint16_t)isr_default,
  /* 93 */ (uint16_t)isr_default,
  /* 92 */ (uint16_t)isr_default,
  /* 91 */ (uint16_t)isr_default,
  /* 90 */ (uint16_t)isr_default,
  /* 89 */ (uint16_t)isr_default,
  /* 88 */ (uint16_t)isr_default,
  /* 87 */ (uint16_t)isr_default,
  /* 86 */ (uint16_t)isr_default,
  /* 85 */ (uint16_t)isr_default,
  /* 84 */ (uint16_t)isr_default,
  /* 83 */ (uint16_t)isr_default,
  /* 82 */ (uint16_t)isr_default,
  /* 81 */ (uint16_t)isr_default,
  /* 80 */ (uint16_t)isr_default,
  /* 79 */ (uint16_t)isr_default,
  /* 78 */ (uint16_t)isr_default,
  /* 77 */ (uint16_t)isr_default,
  /* 76 */ (uint16_t)isr_default,
  /* 75 */ (uint16_t)isr_default,
  /* 74 */ (uint16_t)isr_default,
  /* 73 */ (uint16_t)isr_default,
  /* 72 */ (uint16_t)isr_default,
  /* 71 */ (uint16_t)isr_default,
  /* 70 */ (uint16_t)isr_default,
  /* 69 */ (uint16_t)isr_default,
  /* 68 */ (uint16_t)isr_default,
  /* 67 */ (uint16_t)isr_default,
  /* 66 */ (uint16_t)isr_default,
  /* 65 */ (uint16_t)isr_default,
  /* 64 */ (uint16_t)isr_default,
  /* 63 */ (uint16_t)isr_default,
  /* 62 */ (uint16_t)isr_default,
  /* 61 */ (uint16_t)isr_default,
  /* 60 */ (uint16_t)isr_default,
  /* 59 */ (uint16_t)isr_default,
  /* 58 */ (uint16_t)isr_default,
  /* 57 */ (uint16_t)isr_default,
  /* 56 */ (uint16_t)isr_default,
  /* 55 */ (uint16_t)isr_default,
  /* 54 */ (uint16_t)isr_default,
  /* 53 */ (uint16_t)isr_default,
  /* 52 */ (uint16_t)isr_default,
  /* 51 */ (uint16_t)isr_default,
  /* 50 */ (uint16_t)isr_default,
  /* 49 */ (uint16_t)isr_default,
  /* 48 */ (uint16_t)isr_default,
  /* 47 */ (uint16_t)isr_default,
  /* 46 */ (uint16_t)isr_default,
  /* 45 */ (uint16_t)isr_default,
  /* 44 */ (uint16_t)isr_default,
  /* 43 */ (uint16_t)isr_default,
  /* 42 */ (uint16_t)isr_default,
  /* 41 */ (uint16_t)isr_default,
  /* 40 */ (uint16_t)isr_default,
#ifdef USE_CAN
  /* 39 */ (uint16_t)Can_0_TxIsr_Entry,
  /* 38 */ (uint16_t)Can_0_RxIsr_Entry,
  /* 37 */ (uint16_t)Can_0_ErrIsr_Entry,
  /* 36 */ (uint16_t)Can_0_WakeIsr_Entry,
#else
  /* 39 */ (uint16_t)isr_default,
  /* 38 */ (uint16_t)isr_default,
  /* 37 */ (uint16_t)isr_default,
  /* 36 */ (uint16_t)isr_default,
#endif
  /* 35 */ (uint16_t)isr_default,
  /* 34 */ (uint16_t)isr_default,
  /* 33 */ (uint16_t)isr_default,
  /* 32 */ (uint16_t)isr_default,
#ifdef USE_LIN_SLAVE
  /* 31 */ (uint16_t)Lin_Slave_I2C_ISR,
#else
  /* 31 */ (uint16_t)isr_default,
#endif
  /* 30 */ (uint16_t)isr_default,
  /* 29 */ (uint16_t)isr_default,
  /* 28 */ (uint16_t)isr_default,
  /* 27 */ (uint16_t)isr_default,
  /* 26 */ (uint16_t)isr_default,
  /* 25 */ (uint16_t)isr_default,
  /* 24 */ (uint16_t)isr_default,
  /* 23 */ (uint16_t)isr_default,
  /* 22 */ (uint16_t)isr_default,
  /* 21 */ (uint16_t)isr_default,
  /* 20 */ (uint16_t)isr_default,
  /* 19 */ (uint16_t)isr_default,
  /* 18 */ (uint16_t)isr_default,
  /* 17 */ (uint16_t)isr_default,
  /* 16 */ (uint16_t)isr_default,
  /* 15 */ (uint16_t)isr_default,
  /* 14 */ (uint16_t)isr_default,
  /* 13 */ (uint16_t)isr_default,
  /* 12 */ (uint16_t)isr_default,
  /* 11 */ (uint16_t)isr_default,
  /* 10 */ (uint16_t)isr_default,
  /* 9 */ (uint16_t)isr_default,
  /* 8 */ (uint16_t)isr_default,
  /* 7 */ (uint16_t)Isr_SystemTick,
  /* 6 */ (uint16_t)isr_default,
  /* 5 */ (uint16_t)isr_default,
  /* 4 */ (uint16_t)Isr_SoftwareInterrupt,
  /* 3 */ (uint16_t)isr_default,
  /* 2 */ (uint16_t)isr_default,
  /* 1 */ (uint16_t)isr_default,
  /* 0 */ (uint16_t)isr_default,
#pragma DATA_SEG DEFAULT
};
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt void isr_default(void) {
  while (1)
    ;
}
#pragma CODE_SEG DEFAULT