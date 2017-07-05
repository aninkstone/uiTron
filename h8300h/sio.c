/*
 * sio.c: Interrupt driven serial driver
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

#include "kernel.h"

#ifdef H8_3069
# include "h8_3069.h"
#endif

#define INTERVAL        1000

#define SSR_TDRE        0x80
#define SSR_RDRF        0x40
#define SSR_RDRF_CLEAR  0xbc
#define SSR_TDRE_CLEAR  0x7c
#define SSR_ERROR_CLEAR 0xc4
#define SSR_ERROR       0x38

#define SCR_TIE         0x80
#define SCR_RIE         0x40
#define SCR_TE          0x20
#define SCR_RE          0x10

static void siohdr(INHNO irq);
static const T_DINH dinh = {TA_HLNG, (FP)siohdr, 0x8000};

typedef struct {
    int bps;
    unsigned char smr;
    unsigned char brr;
} T_SIO_SPD;

static const T_SIO_SPD ini_sio_spd[] = {
    /* for CPU CLK 20MHz */
    {57600, 0,  10},
    {38400, 0,  15},
    {19200, 0,  32},
    { 9600, 0,  64},
    { 4800, 0, 129},
    { 2400, 1,  64},
    { 1200, 1, 129},
    {  600, 2,  64},
    {  300, 2, 129},
    {  150, 3,  64},
    {  110, 3,  88},
    {    0, 0,   0}
};

typedef struct {
    int ch;
    volatile unsigned char *scr;
    volatile unsigned char *smr;
    volatile unsigned char *ssr;
    volatile unsigned char *brr;
    volatile unsigned char *rdr;
    volatile unsigned char *tdr;
    INHNO txint;
    INHNO rxint;
    INHNO erint;
} T_SIO_CTL;

static const T_SIO_CTL sio_ctl[3] = {
    {0, &SCR0, &SMR0, &SSR0, &BRR0, &RDR0, &TDR0, INT_TXI0, INT_RXI0, INT_ERI0},
    {1, &SCR1, &SMR1, &SSR1, &BRR1, &RDR1, &TDR1, INT_TXI1, INT_RXI1, INT_ERI1},
    {2, &SCR2, &SMR2, &SSR2, &BRR2, &RDR2, &TDR2, INT_TXI2, INT_RXI2, INT_ERI2}
};

static ID sdtq[3];
static ID rdtq[3];

/*
 * SIO interrupt handler (INH)
 */
static void siohdr(INHNO irq)
{
    T_SIO_CTL *ctl;
    unsigned char ssr;
    VP_INT data;
    
    if (irq == INT_RXI0 || irq == INT_ERI0 || irq == INT_TXI0) {
        ctl = (T_SIO_CTL *)&sio_ctl[0];
    } else if (irq == INT_RXI1 || irq == INT_ERI1 || irq == INT_TXI1) {
        ctl = (T_SIO_CTL *)&sio_ctl[1];
    } else {
        ctl = (T_SIO_CTL *)&sio_ctl[2];
    }
    
    ssr = *ctl->ssr;
retry:
    if (ssr & SSR_ERROR) {
        /*
         * clear error
         */
        *ctl->ssr = SSR_RDRF_CLEAR & SSR_ERROR_CLEAR;
    } else if (ssr & SSR_RDRF) {
        /*
         * receive
         */
        data = (VP_INT)(unsigned int)*ctl->rdr;
        *ctl->ssr = SSR_RDRF_CLEAR & SSR_ERROR_CLEAR;
        psnd_dtq(rdtq[ctl->ch], data);
    }
    
    if (*ctl->scr & SCR_TIE) {
        /*
         * send
         */
        ssr = *ctl->ssr;
        if (ssr & SSR_TDRE) {
            if (prcv_dtq(sdtq[ctl->ch], &data) == E_OK) {
                *ctl->tdr = (unsigned char)(unsigned int)data;
                *ctl->ssr = SSR_TDRE_CLEAR & SSR_ERROR_CLEAR;
            } else {
                *ctl->scr = SCR_RIE | SCR_TE | SCR_RE; /* TIE OFF */
            }
        }
    }

    /*
     * retry receive
     */
    ssr = *ctl->ssr;
    if (ssr & (SSR_RDRF | SSR_ERROR))
        goto retry;
}

/*
 * put a character to SIO
 *   ch: 0,1,2
 *   data: character
 */
void put_sio(int ch, int data)
{
    T_SIO_CTL *ctl;
    
    if (ch == 0) {
        ctl = (T_SIO_CTL *)&sio_ctl[0];
    } else if (ch == 1) {
        ctl = (T_SIO_CTL *)&sio_ctl[1];
    } else {
        ctl = (T_SIO_CTL *)&sio_ctl[2];
    }
    snd_dtq(sdtq[ctl->ch], (VP_INT)data);
    *ctl->scr = SCR_TIE | SCR_RIE | SCR_TE | SCR_RE; /* TIE ON */
}

/*
 * get a character from SIO
 *   ch: 0,1,2
 *   tmout: time out value or TMO_FEVR or TMO_POL
 *   return value:
 *     >= 0: received character
 *     <  0: trcv_dtq error code
 */
int get_sio(int ch, TMO tmout)
{
    T_SIO_CTL *ctl;
    VP_INT data;
    ER_UINT r;
    
    if (ch == 0) {
        ctl = (T_SIO_CTL *)&sio_ctl[0];
    } else if (ch == 1) {
        ctl = (T_SIO_CTL *)&sio_ctl[1];
    } else {
        ctl = (T_SIO_CTL *)&sio_ctl[2];
    }
    if ((r = trcv_dtq(rdtq[ctl->ch], &data, tmout)) == E_OK)
        r = (int)data;
    return r;
}

/*
 * Initiallize SIO driver
 *   ch    : 0,1,2
 *   bps   : 57600 - 110
 *   option:
 *     SIO_7BIT: 7 data bits
 *     SIO_PTY : enable parity
 *     SIO_ODD : odd parity
 *     SIO_2STP: 2 stop bits
 *     (default: 8 data bits, no parity, 1 stop bit)
 *   return value:
 *     E_OK : OK
 *     -1   : bad bps
 *     other: def_inh/acre_dtq error return value
 */

ER ini_sio(int ch, int bps, int option, UINT bufsiz)
{
    T_SIO_SPD *spd;
    T_SIO_CTL *ctl;
    volatile int i;
    ER r = E_OK;
    ER_ID id;
    T_CDTQ cdtq;

    for (spd = (T_SIO_SPD *)&ini_sio_spd[0];
         spd->bps && spd->bps != bps; spd++);
    if (!spd->bps) {
        r = -1;                 /* bps parameter error */
        goto ret;
    }

    if (ch == 0) {
        ctl = (T_SIO_CTL *)&sio_ctl[0];
    } else if (ch == 1) {
        ctl = (T_SIO_CTL *)&sio_ctl[1];
    } else {
        ctl = (T_SIO_CTL *)&sio_ctl[2];
    }

    if ((r = def_inh(ctl->txint, &dinh)) < 0)
        goto ret;
    if ((r = def_inh(ctl->rxint, &dinh)) < 0)
        goto ret;
    if ((r = def_inh(ctl->erint, &dinh)) < 0)
        goto ret;

#ifdef TA_BYTE
    cdtq.dtqatr = TA_TPRI | TA_BYTE;
#else
    cdtq.dtqatr = TA_TPRI;
#endif
    cdtq.dtqcnt = bufsiz;
    cdtq.dtq    = NULL;
    if ((id = acre_dtq(&cdtq)) < 0) {
        r = (ER)id;
        goto ret;
    }
    sdtq[ctl->ch] = (ID)id;
    if ((id = acre_dtq(&cdtq)) < 0) {
        del_dtq(sdtq[ctl->ch]);
        r = (ER)id;
        goto ret;
    }
    rdtq[ctl->ch] = (ID)id;

    *ctl->scr = 0x00;           /* STOP SCI */  
    *ctl->smr = option | spd->smr;
    *ctl->brr = spd->brr;
    for (i = INTERVAL; i--;);   /* WAIT 1 BIT CYCLE */
    *ctl->scr = SCR_RIE | SCR_TE | SCR_RE;
ret:
    return r;
}

/* end */
