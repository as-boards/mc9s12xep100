/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2019-2023 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include "Mcu.h"
#include <hidef.h>
#include "derivative.h"
#include "Os.h"
#include "Std_Critical.h"
#include "Std_Timer.h"
#include "Flash.h"
#include "ringbuffer.h"
/* ================================ [ MACROS    ] ============================================== */
#define CPU_FREQUENCY 32000000
#ifdef USE_CLIB_STDIO_PRINTF
#define TERMIO_PutChar __putchar
#endif

#define LEDCPU PORTK_PK4
#define LEDCPU_dir DDRK_DDRK4

#ifdef USE_SDCARD
#define CD PTIJ_PTIJ0
#define CD_dir DDRJ_DDRJ0
#define WP PTIJ_PTIJ1
#define WP_dir DDRJ_DDRJ1

#define DDR_INI() DDRS |= 0xe0
#define SD_select() PTS_PTS7 = 0
#define SD_deselect() PTS_PTS7 = 1
#define PTS_INIT() DDRS |= 0xe0
#endif

#define ISRNO_VRTI VectorNumber_Vrti
#define ISRNO_VSWI VectorNumber_Vswi

#define IS_FLASH_ADDRESS(a)                                                                        \
  ((((a) <= 0x8000) && ((a) >= 0x4000)) || (((a) <= 0xE000) && ((a) >= 0xC000)) ||                 \
   ((((a) >> 16) <= 0xFC) && (((a) >> 16) >= 0xC0) && (((a)&0xFFFF) <= 0xC000) &&                  \
    (((a)&0xFFFF) >= 0x8000)))

/* ================================ [ TYPES     ] ============================================== */
typedef __far void (*FP)();
typedef struct {
  uint16 tag; /* 2 bytes */
  FP entry;   /* 3 bytes */
  uint8 reserved;
} AppTag_Type;
/* ================================ [ DECLARES  ] ============================================== */
extern void OsTick(void);

void StartOsTick(void);

#if !defined(__AS_BOOTLOADER__)
extern const FP tisr_pc[];
#endif

#ifdef USE_OS
extern void EnterISR(void);
extern void LeaveISR(void);
#endif

extern void _Startup(void);
/* ================================ [ DATAS     ] ============================================== */
#if !defined(__AS_BOOTLOADER__)
#pragma DATA_SEG __NEAR_SEG APP_TAG
const AppTag_Type app_tag = {0xA55A, _Startup, 0x5A};
#pragma DATA_SEG __NEAR_SEG DEFAULT
#endif

#ifdef USE_OS
extern uint32_t OsTickCounter;
#else
uint32_t OsTickCounter = 0;
#endif

RB_DECLARE(stdio_out, char, 1024);
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
void TERMIO_PutChar(char c) {
  EnterCritical();
  RB_PUSH(stdio_out, &c, 1);
  ExitCritical();
}

void Mcu_Init(const Mcu_ConfigType *configPtr) {
  (void *)configPtr;

#if !defined(__AS_BOOTLOADER__)
  {
    volatile uint16 tag;
    tag = app_tag.tag; /* access to keep the app tag */
  }
  IVBR = ((uint16)tisr_pc) >> 8;
#endif

  CLKSEL &= 0x7f; // set OSCCLK as sysclk
  PLLCTL &= 0x8F; // Disable PLL circuit
  CRGINT &= 0xDF;

#if (CPU_FREQUENCY == 40000000)
  SYNR = 0x44;
#elif (CPU_FREQUENCY == 32000000)
  SYNR = 0x43;
#elif (CPU_FREQUENCY == 24000000)
  SYNR = 0x42;
#endif

  REFDV = 0x81;
  PLLCTL = PLLCTL | 0x70; // Enable PLL circuit
  asm NOP;
  asm NOP;
  while (!(CRGFLG & 0x08))
    ;             // PLLCLK is Locked already
  CLKSEL |= 0x80; // set PLLCLK as sysclk

  /* Init UART for debug */
  SCI0BD = CPU_FREQUENCY / 16 / 115200;
  SCI0CR1 = 0x00;
  SCI0CR2 = 0x0C; /* enable RX and TX, no interrupt */

  LEDCPU_dir = 1;

#ifndef USE_OS
  StartOsTick();
#endif

  asm cli; // enable ISR

  printf("MC9S12 is ready\n");
}

uint32_t McuE_GetSystemClock(void) {
  return (uint32_t)CPU_FREQUENCY;
}

void Mcu_PerformReset(void) {
  COPCTL = 0x41; /* enable the COP */
  while (1)
    ;
}

void Dcm_PerformReset(uint8_t resetType) {
  // Mcu_PerformReset();
  printf("skip reset\n");
}

void stdio_main_function(void) {
  uint8_t c;
  rb_size_t sz;
  if (SCI0SR1_TDRE) {
    EnterCritical();
    sz = RB_POP(stdio_out, &c, 1);
    ExitCritical();
    if (sz > 0) {
      SCI0DRL = c;
    }
  }

#ifdef USE_SHELL
  if (SCI0SR1_RDRF) {
    char ch = SCI0DRL;
    if ('\r' == ch)
      ch = '\n';
    SHELL_input(ch);
  }
#endif
}

#ifdef USE_BL
void BL_AliveIndicate(void) {
  LEDCPU = ~LEDCPU;
}
#endif

#ifdef USE_SDCARD
void sd_spi_init(int sd) {
  DDRS = 0xE0;
  SPI0CR2 = 0x10;
  SPI0CR1 = 0x5e;
  SPI0BR = 0x45; /* set baudrate 100k */
  SD_deselect();
  CD_dir = 0;
  WP_dir = 0;
}

int sd_spi_transmit(int sd, const uint8_t *txData, uint8_t *rxData, size_t size) {
  int ercd = 0;
  TimerType timer;
  TickType timeout;
  uint8_t u8Value;

  timeout = size;
  StartTimer(&timer);
  while ((size > 0) && (GetTimer(&timer) <= (MS2TICKS(timeout)))) {
    if (txData != NULL) {
      u8Value = *txData;
      txData++;
    } else {
      u8Value = 0xFF;
    }
    while ((0 == SPI0SR_SPTEF) && (GetTimer(&timer) <= (MS2TICKS(timeout))))
      ;
    SPI0DR = u8Value;
    while ((0 == SPI0SR_SPIF) && (GetTimer(&timer) <= (MS2TICKS(timeout))))
      ;
    u8Value = SPI0DR;

    if (rxData != NULL) {
      *rxData = u8Value;
      rxData++;
    }
    size--;
  }

  if (GetTimer(&timer) > (MS2TICKS(timeout))) {
    ercd = ETIMEDOUT;
  }

  return ercd;
}

void sd_chip_selected(int sd, int select) {
  PTS_PTS7 = select;
}

int sd_is_detected(int sd) {
  int rv;

  if (1 == CD) {
    rv = FALSE;
  } else {
    rv == TRUE;
  }

  return rv;
}

void sd_spi_clk_slow(int sd) {
  SPI0BR = 0x45; /* set baudrate 100k */
}

void sd_spi_fast_fast(int sd) {
  SPI0BR = 0x11; /* set baudrate 4M */
}
#endif

imask_t Std_EnterCritical(void) {
  asm psha;
  asm tpa;
  asm tab;
  asm sei;
  asm pula;
}

void Std_ExitCritical(imask_t mask) {
  (void)mask;
  asm psha;
  asm tba;
  asm tap;
  asm pula;
}

void Irq_Enable(void) {
  asm cli;
}

void Irq_Disable(void) {
  asm sei;
}

/*
#python code to search the best for the given expect period
OSCLK = 16*1000000
expect = 1/100
diff = OSCLK
best = None
# RTDEC = 0
for rtr30 in range(16):
  for rtr64 in range(8):
    period = (rtr30+1)*2**(rtr64+9)/OSCLK
    ndiff = abs(period-expect)
    if(ndiff < diff):
      diff = ndiff
      best = (0, rtr64, rtr30, period, diff)
# RTDEC = 1
TABLE = [1,2,5,10,20,50,100,200]
for rtr30 in range(16):
  for rtr64 in range(8):
    period = (rtr30+1)*TABLE[rtr64]*1000/OSCLK
    ndiff = abs(period-expect)
    if(ndiff < diff):
      diff = ndiff
      best = (1, rtr64, rtr30, period, diff)
print('best is', hex((best[0]<<7)+(best[1]<<4)+best[2]))
 */
void StartOsTick(void) {
#if OS_TICKS_PER_SECOND == 100
  RTICTL = 0xc7; /* period is 10ms */
#else
#error wrong configuration of OS_TICKS_PER_SECOND
#endif
  CRGINT_RTIE = 1; /* enable real-time interrupt */
}

#ifndef USE_OS
void OsTick(void) {
  OsTickCounter++;
}
#endif

std_time_t Std_GetTime(void) {
  return OsTickCounter * (1000000 / OS_TICKS_PER_SECOND);
}

#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt ISRNO_VRTI void Isr_SystemTick(void) {
#ifdef USE_OS
  EnterISR();
#endif
  CRGFLG &= 0xEF; // clear the interrupt flag
  OsTick();
#ifdef USE_OS
  SignalCounter(0);
#endif
#ifdef USE_OS
  LeaveISR();
#endif
}

#if !defined(USE_OS)
interrupt ISRNO_VSWI void Isr_SoftwareInterrupt(void) {
}
#endif

#ifdef BL_USE_BUILTIN_FLS_READ
void FlashRead(tFlashParam *FlashParam) {
  tAddress address;
  tLength length;
  tData *data;

  length = FlashParam->length;
  address = FlashParam->address;
  data = FlashParam->data;
  if (FALSE == IS_FLASH_ADDRESS(address)) {
    FlashParam->errorcode = kFlashInvalidAddress;
  } else if (FALSE == IS_FLASH_ADDRESS(address + length)) {
    FlashParam->errorcode = kFlashInvalidSize;
  } else if ((address & 0xFF0000) != ((address + length) & 0xFF0000)) {
    /* not in the same page */
    FlashParam->errorcode = kFlashInvalidSize;
  } else if (NULL == data) {
    FlashParam->errorcode = kFlashInvalidData;
  } else {
    tLength i;
    uint8_t savedPPAGE = PPAGE;
    char *dst = (char *)data;
    const char *src = (const char *)address;

    EnterCritical();
    PPAGE = address >> 16;
    for (i = 0; i < length; i++) {
      dst[i] = src[i];
    }
    PPAGE = savedPPAGE;
    ExitCritical();

    FlashParam->errorcode = kFlashOk;
  }
}
#endif
#pragma CODE_SEG DEFAULT
