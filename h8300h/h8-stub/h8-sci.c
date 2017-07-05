#include "H8_3069F.h"

#define INTERVAL		1000

#define SCI_TDRE		0x80
#define SCI_RDRF		0x40
#define SCI_BRK			0x20
#define SCI_FER			0x10
#define SCI_PER			0x08

#define SCI_RDRF_CLEAR		0xBC
#define SCI_TDRE_CLEAR		0x78
#define SCI_ERROR_CLEAR		0xC4

/* SELECT SIO CH (0/1/2) */
#ifndef CH
# define CH 1
#endif

#if CH == 0
# define SCR SCR0
# define SMR SMR0
# define SSR SSR0
# define BRR BRR0
# define RDR RDR0
# define TDR TDR0
#elif CH == 1
# define SCR SCR1
# define SMR SMR1
# define SSR SSR1
# define BRR BRR1
# define RDR RDR1
# define TDR TDR1
#elif CH == 2
# define SCR SCR2
# define SMR SMR2
# define SSR SSR2
# define BRR BRR2
# define RDR RDR2
# define TDR TDR2
#else
# error
#endif

void
handleError (void)
{
  SSR = SCI_ERROR_CLEAR;
}

void
init_serial(void)
{
  int i = INTERVAL;

  SCR = 0x00;		/* TE/RE CLEAR */  
  SMR = 0x00;		/* ASYNC 8 BIT, NON PARITY, 1 STOP BIT */
//  BRR = 10;		/* 57600 BPS (20MHz) */
  BRR = 15;		/* 38400 BPS (20MHz) */
//  BRR = 32;		/* 19200 BPS (20MHz) */
  while (i--);		/* WAIT 1 BIT CYCLE */
  SCR = 0x70;		/* RIE/TE/RE SET */

/* SCIn Interrupt is High Level Priority */
#if CH == 0
  IPRB |= 0x08;
#elif CH == 1
  IPRB |= 0x04;
#elif CH == 2
  IPRB |= 0x02;
#else
# error
#endif
}

static inline int
getDebugCharReady (void)
{
  unsigned char status;

  status = SSR;
  if (status & (SCI_PER | SCI_FER | SCI_BRK))	/* ERROR ? */
    handleError ();

  return (status & SCI_RDRF);
}

char 
getDebugChar (void)
{
  unsigned char status;
  char ch;

  while ( ! getDebugCharReady());

  ch = RDR;
  status = SSR;
  SSR = SCI_RDRF_CLEAR; 

  if (status & (SCI_PER | SCI_FER | SCI_BRK))	/* ERROR ? */
    handleError ();

  return ch;
}

static inline int 
putDebugCharReady (void)
{
  unsigned char status;

  status = SSR;
  return (status & SCI_TDRE);
}

void
putDebugChar (char ch)
{
    
  while (!putDebugCharReady());

  TDR = ch;
  SSR = SCI_TDRE_CLEAR;
}
