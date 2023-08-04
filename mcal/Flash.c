/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2019-2023 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include "Std_Types.h"
#include <hidef.h>
#include "derivative.h"
#define FLASH_IS_ERASE_ADDRESS_ALIGNED(a) ((0 == ((a)&0x3FF)))

#ifndef FLASH_WRITE_SIZE
#define FLASH_WRITE_SIZE 8
#endif

#include "Flash.h"
/* ================================ [ MACROS    ] ============================================== */
#if FLASH_WRITE_SIZE % 8 != 0
#error FLASH_WRITE_SIZE must be n times of 8
#endif

#define IS_FLASH_ADDRESS(a)                                                                        \
  ((((a) <= 0x8000) && ((a) >= 0x4000)) || (((a) <= 0xE000) && ((a) >= 0xC000)) ||                 \
   ((((a) >> 16) <= 0xFC) && (((a) >> 16) >= 0xC0) && (((a)&0xFFFF) <= 0xC000) &&                  \
    (((a)&0xFFFF) >= 0x8000)))

#define AS_LOG_FLS 0

#define FlashHeader _Startup

/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
const tFlashHeader FlashHeader = {
  /* .Info */ 1 + (2 << 8) + (169 << 24),
  /*.Init      =*/FlashInit,
  /*.Deinit    =*/FlashDeinit,
  /*.Erase     =*/FlashErase,
  /*.Write     =*/FlashWrite,
  /*.Read      =*/FlashRead};
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
void FlashInit(tFlashParam *FlashParam) {
  if ((FLASH_DRIVER_VERSION_PATCH == FlashParam->patchlevel) ||
      (FLASH_DRIVER_VERSION_MINOR == FlashParam->minornumber) ||
      (FLASH_DRIVER_VERSION_MAJOR == FlashParam->majornumber)) {
    while (FSTAT_CCIF == 0)
      ;
    FCLKDIV = 0x0F; /* set FCLK to 1MHZ OSCCLK/16 */
    FCNFG = 0x00;
    while (FCLKDIV_FDIVLD == 0)
      ;
    FlashParam->errorcode = kFlashOk;
  } else {
    FlashParam->errorcode = kFlashFailed;
  }
}

void FlashDeinit(tFlashParam *FlashParam) {
  /*  TODO: Deinit Flash Controllor */
  FlashParam->errorcode = kFlashOk;
}

void FlashErase(tFlashParam *FlashParam) {
  tAddress address;
  tAddress globalAddress;
  tLength length;
  if ((FLASH_DRIVER_VERSION_PATCH == FlashParam->patchlevel) ||
      (FLASH_DRIVER_VERSION_MINOR == FlashParam->minornumber) ||
      (FLASH_DRIVER_VERSION_MAJOR == FlashParam->majornumber)) {
    length = FlashParam->length;
    address = FlashParam->address;
    if ((FALSE == FLASH_IS_ERASE_ADDRESS_ALIGNED(address)) ||
        (FALSE == IS_FLASH_ADDRESS(address))) {
      FlashParam->errorcode = kFlashInvalidAddress;
    } else if ((FALSE == IS_FLASH_ADDRESS(address + length)) ||
               (FALSE == FLASH_IS_ERASE_ADDRESS_ALIGNED(address + length))) {
      FlashParam->errorcode = kFlashInvalidSize;
    } else if ((address & 0xFF0000) != ((address + length - 1) & 0xFF0000)) {
      /* not in the same page */
      FlashParam->errorcode = kFlashInvalidSize;
    } else {
      while ((length > 0) && (kFlashOk == FlashParam->errorcode)) {
        if (address & 0xFF0000) {
          globalAddress = (address >> 16) * 16 * 1024 + 0x400000 + (address & 0x3FFF);
        } else {
          globalAddress = 0x7F0000 + address;
        }

        while (FSTAT_CCIF == 0)
          ;
        if (FSTAT_ACCERR)
          FSTAT_ACCERR = 1;
        if (FSTAT_FPVIOL)
          FSTAT_FPVIOL = 1;

        FCCOBIX_CCOBIX = 0x00;
        FCCOB = (0x0A << 8) + (globalAddress >> 16); /* Erase P-Flash Sector */
        FCCOBIX_CCOBIX = 0x01;
        FCCOB = globalAddress & 0xFFFF;
        FSTAT_CCIF = 1;
        while (FSTAT_CCIF == 0)
          ;

        if (FSTAT_ACCERR || FSTAT_FPVIOL) {
          FlashParam->errorcode = FSTAT_ACCERR ? kFlashAccerr : kFlashPViol;
          FlashParam->errorAddress = address;
        } else {
          uint8 savedPage = PPAGE;
          tAddress addr;

          PPAGE = (address >> 16) & 0xFF;
          for (addr = address; addr < (address + 1024); addr += 4) {
            uint16 addrInWindow = addr & 0xFFFF;
            if ((*(volatile uint32 *)addrInWindow) != 0xFFFFFFFF) {
              FlashParam->errorcode = kFlashFailed;
              break;
            }
          }
          PPAGE = savedPage;

          address += 1024;
          length -= 1024;
        }
      }
    }
  } else {
    FlashParam->errorcode = kFlashFailed;
  }
}

void FlashWrite(tFlashParam *FlashParam) {
  tAddress address;
  tAddress globalAddress;
  tLength length;
  tData *data;
  if ((FLASH_DRIVER_VERSION_PATCH == FlashParam->patchlevel) ||
      (FLASH_DRIVER_VERSION_MINOR == FlashParam->minornumber) ||
      (FLASH_DRIVER_VERSION_MAJOR == FlashParam->majornumber)) {
    length = FlashParam->length;
    address = FlashParam->address;
    data = FlashParam->data;
    if ((FALSE == FLASH_IS_WRITE_ADDRESS_ALIGNED(address)) ||
        (FALSE == IS_FLASH_ADDRESS(address))) {
      FlashParam->errorcode = kFlashInvalidAddress;
    } else if ((FALSE == IS_FLASH_ADDRESS(address + length)) ||
               (FALSE == FLASH_IS_WRITE_ADDRESS_ALIGNED(length))) {
      FlashParam->errorcode = kFlashInvalidSize;
    } else if ((address & 0xFF0000) != ((address + length - 1) & 0xFF0000)) {
      /* not in the same page */
      FlashParam->errorcode = kFlashInvalidSize;
    } else if (NULL == data) {
      FlashParam->errorcode = kFlashInvalidData;
    } else {
      while ((length > 0) && (kFlashOk == FlashParam->errorcode)) {
        if (0 != (address & 0xFF0000)) {
          globalAddress = (address >> 16) * 16 * 1024 + 0x400000 + (address & 0x3FFF);
        } else {
          globalAddress = 0x7F0000 + address;
        }

        while (FSTAT_CCIF == 0)
          ;
        if (FSTAT_ACCERR)
          FSTAT_ACCERR = 1;
        if (FSTAT_FPVIOL)
          FSTAT_FPVIOL = 1;
        FCCOBIX_CCOBIX = 0x00;
        FCCOB = (0x06 << 8) + (globalAddress >> 16); /* Program P-Flash */
        FCCOBIX_CCOBIX = 0x01;
        FCCOB = globalAddress & 0xFFFF;
        FCCOBIX_CCOBIX = 0x02;
        FCCOB = ((uint16_t *)data)[0];
        FCCOBIX_CCOBIX = 0x03;
        FCCOB = ((uint16_t *)data)[1];
        FCCOBIX_CCOBIX = 0x04;
        FCCOB = ((uint16_t *)data)[2];
        FCCOBIX_CCOBIX = 0x05;
        FCCOB = ((uint16_t *)data)[3];

        FSTAT_CCIF = 1;
        while (FSTAT_CCIF == 0)
          ;

        if (FSTAT_ACCERR || FSTAT_FPVIOL) {
          FlashParam->errorcode = FSTAT_ACCERR ? kFlashAccerr : kFlashPViol;
          FlashParam->errorAddress = address;
        } else {
          uint8 savedPage = PPAGE;
          uint16 addrInWindow = address & 0xFFFF;

          PPAGE = (address >> 16) & 0xFF;
          if (((*(volatile uint32 *)addrInWindow) != ((volatile uint32 *)data)[0]) ||
              ((*(volatile uint32 *)(addrInWindow + 4)) != ((volatile uint32 *)data)[1])) {
            FlashParam->errorcode = kFlashFailed;
            FlashParam->errorAddress = address;
          }
          PPAGE = savedPage;

          data += 8;
          address += 8;
          length -= 8;
        }
      }
    }
  } else {
    FlashParam->errorcode = kFlashFailed;
  }
}

void FlashRead(tFlashParam *FlashParam) {
  tAddress address;
  tLength length;
  tData *data;
  if ((FLASH_DRIVER_VERSION_PATCH == FlashParam->patchlevel) ||
      (FLASH_DRIVER_VERSION_MINOR == FlashParam->minornumber) ||
      (FLASH_DRIVER_VERSION_MAJOR == FlashParam->majornumber)) {
    length = FlashParam->length;
    address = FlashParam->address;
    data = FlashParam->data;
    if ((FALSE == FLASH_IS_READ_ADDRESS_ALIGNED(address)) || (FALSE == IS_FLASH_ADDRESS(address))) {
      FlashParam->errorcode = kFlashInvalidAddress;
    } else if ((FALSE == IS_FLASH_ADDRESS(address + length)) ||
               (FALSE == FLASH_IS_READ_ADDRESS_ALIGNED(length))) {
      FlashParam->errorcode = kFlashInvalidSize;
    } else if (NULL == data) {
      FlashParam->errorcode = kFlashInvalidData;
    } else {
      tLength i;
      uint8_t savedPPAGE = PPAGE;
      char *dst = (char *)data;
      const char *src = (const char *)address;

      PPAGE = address >> 16;
      for (i = 0; i < length; i++) {
        dst[i] = src[i];
      }

      PPAGE = savedPPAGE;
      FlashParam->errorcode = kFlashOk;
    }
  } else {
    FlashParam->errorcode = kFlashFailed;
  }
}
