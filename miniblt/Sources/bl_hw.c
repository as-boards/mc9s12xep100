/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2019-2023 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */
#include "Mcu.h"
#include "Can.h"
#include "Flash.h"
#include "Dcm.h"
#include <stdio.h>
/* ================================ [ MACROS    ] ============================================== */
#define APP_TAG_ADDR 0xC000

#define F1 PTIH_PTIH4
#define F1_dir DDRH_DDRH4
/* ================================ [ TYPES     ] ============================================== */
typedef __far void (*FP)();
typedef struct {
  uint16 tag; /* 2 bytes */
  FP entry;   /* 3 bytes */
  uint8 reserved;
} AppTag_Type;
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
#pragma DATA_SEG __NEAR_SEG FLASH_RAM
tFlashHeader FlashDriverRam @0x3900;
#pragma DATA_SEG DEFAULT
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
Std_ReturnType BL_IsAppValid(void) {
  Std_ReturnType ret = E_NOT_OK;
  AppTag_Type *appTag = (AppTag_Type *)APP_TAG_ADDR;

  if ((0xA55A == appTag->tag) && (0x5A == appTag->reserved) && (NULL != appTag->entry)) {
    ret = E_OK;
  }
  return ret;
}

void application_main(void) {
  AppTag_Type *appTag = (AppTag_Type *)APP_TAG_ADDR;

  if ((0xA55A == appTag->tag) && (0x5A == appTag->reserved) && (NULL != appTag->entry)) {
    CRGINT_RTIE = 0; /* stop os tick ISR */
    appTag->entry();
  }
}

void BL_JumpToApp(void) {
  application_main();
}

boolean BL_IsUpdateRequested(void) {
  boolean r = FALSE;
  uint32_t *magic = (uint32_t *)&FlashHeader;
  
  F1_dir = 0;
  if (0x12345678 == *magic) {
    r = TRUE;
  }

  if (0 == F1) {
    r = TRUE;
  }

  return r;
}

Std_ReturnType BL_GetProgramCondition(Dcm_ProgConditionsType **cond) {
  Std_ReturnType r = E_NOT_OK;
  uint32_t *magic = (uint32_t *)&FlashHeader;

  if (0x12345678 == *magic) {
    r = E_OK;
    *cond = (Dcm_ProgConditionsType *)(&magic[1]);
  }

  return r;
}
