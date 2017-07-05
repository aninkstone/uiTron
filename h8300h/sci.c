/*
 * sci.c : sci get/put functions for debug
 *
 * Copyright (C) 2007,2008,2009,2010
 * 256TECH Co., Ltd.
 * Masahiro Sakamoto (m-sakamoto@users.sourceforge.net)
 *
 * This file is part of URIBO.
 *
 * URIBO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * URIBO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with URIBO; see the file COPYING and COPYING.LESSER.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef H8_3069
# include "h8_3069.h"
#endif

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
# error : Bad CH
#endif

#define INTERVAL		1000
#define SCI_TDRE		0x80
#define SCI_RDRF		0x40
#define SCI_RDRF_CLEAR		0xBC
#define SCI_TDRE_CLEAR		0x7C
#define SCI_ERROR_CLEAR		0xC4
#define SCI_ERROR               0x38

int get_sci()
{
    unsigned char ssr;
    unsigned int c;

    for (;;) {
        ssr = SSR;
        if (ssr & SCI_ERROR)
            SSR = SCI_ERROR_CLEAR;
        if (ssr & SCI_RDRF)
            break;
    }
    c = RDR;
    SSR = SCI_RDRF_CLEAR; 
    return (int)c;
}

void put_sci(char c)
{
    while (!(SSR & SCI_TDRE))
        ;
    TDR = c;
    SSR = SCI_TDRE_CLEAR;
}

void ini_sci(void)
{
  volatile int i = INTERVAL;

  SCR = 0x00;		/* TE/RE CLEAR */  
  SMR = 0x00;		/* ASYNC 8 BIT, NON PARITY, 1 STOP BIT */
//  BRR = 10;		/* 57600 BPS (20MHz) */
  BRR = 15;		/* 38400 BPS (20MHz) */
//  BRR = 32;		/* 19200 BPS (20MHz) */
  while (i--);		/* WAIT 1 BIT CYCLE */
  SCR = 0x30;		/* TE/RE SET */
}

/* end */
