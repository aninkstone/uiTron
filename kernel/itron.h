#ifndef _ITRON_H_
#define _ITRON_H_
/*
 * itron.h : Define data of uITRON4.0 specification
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

#include "itron_s.h"

#define TKERNEL_MAKER 0x0008
#define TKERNEL_PRID  0x0001
#define TKERNEL_SPVER 0x5403
#define TKERNEL_PRVER 0x0008

#define NULL  0
#define TRUE  1
#define FALSE 0
#define E_OK  0

#define TA_NULL    0
#define TA_HLNG    0x00
#define TA_ASM     0x01
#define TA_TFIFO   0x00
#define TA_TPRI    0x01
#define TA_TPRIR   0x04
#define TA_MFIFO   0x00
#define TA_MPRI    0x02
#define TA_ACT     0x02
#define TA_RSTR    0x04
#define TA_WSGL    0x00
#define TA_WMUL    0x02
#define TA_CLR     0x04
#define TA_INHERIT 0x02
#define TA_CEILING 0x03
#define TA_STA     0x02
#define TA_PHS     0x04
#define TA_BYTE    0x08         /* extended from uITRON */

#define TMO_POL  0
#define TMO_FEVR ((TMO)-1)
#define TMO_NBLK ((TMO)-2)

#define TWF_ANDW 0x00
#define TWF_ORW  0x01
#define TWF_CLR  0x04

#define TTS_RUN 0x01
#define TTS_RDY 0x02
#define TTS_WAI 0x04
#define TTS_SUS 0x08
#define TTS_WAS 0x0c
#define TTS_DMT 0x10

#define TTW_SLP  0x0001
#define TTW_DLY  0x0002
#define TTW_SEM  0x0004
#define TTW_FLG  0x0008
#define TTW_SDTQ 0x0010
#define TTW_RDTQ 0x0020
#define TTW_MBX  0x0040
#define TTW_MTX  0x0080
#define TTW_SMBF 0x0100
#define TTW_RMBF 0x0200
#define TTW_MBF  0x0200
#define TTW_CAL  0x0400
#define TTW_ACP  0x0800
#define TTW_RDV  0x1000
#define TTW_MPF  0x2000
#define TTW_MPL  0x4000

#define TTEX_ENA 0x0000
#define TTEX_DIS 0x0001

#define TCYC_STP 0x0000
#define TCYC_STA 0x0001

#define TALM_STP 0x0000
#define TALM_STA 0x0001

#define TOVR_STP 0x0000
#define TOVR_STA 0x0001

#define TSK_SELF 0
#define TSK_NONE 0

#define TPRI_SELF 0
#define TPRI_INI  0

#define ERCD(mercd,sercd) (mercd)
#define MERCD(ercd) (ercd)
#define SERCD(ercd) ((ER)-1)

#define TMIN_TPRI 1
#define TMAX_TPRI (_kernel_tpri_max)
#define TMIN_MPRI 1
#define TMAX_MPRI TMAX_INT

# define TMAX_ACTCNT 255
# define TMAX_WUPCNT 255
# define TMAX_SUSCNT 255
# define TMAX_MAXSEM 65535

#define TBIT_TEXPTN (sizeof(TEXPTN)*8)
#define TBIT_FLGPTN (sizeof(FLGPTN)*8)
#define TBIT_RDVPTN (sizeof(RDVPTN)*8)

#define E_SYS   ((ER)-5)
#define E_NOSPT ((ER)-9)
#define E_RSFN  ((ER)-10)
#define E_RSATR ((ER)-11)
#define E_PAR   ((ER)-17)
#define E_ID    ((ER)-18)
#define E_CTX   ((ER)-25)
#define E_MACV  ((ER)-26)
#define E_OACV  ((ER)-27)
#define E_ILUSE ((ER)-28)
#define E_NOMEM ((ER)-33)
#define E_NOID  ((ER)-34)
#define E_OBJ   ((ER)-41)
#define E_NOEXS ((ER)-42)
#define E_QOVR  ((ER)-43)
#define E_RLWAI ((ER)-49)
#define E_TMOUT ((ER)-50)
#define E_DLT   ((ER)-51)
#define E_CLS   ((ER)-52)
#define E_WBLK  ((ER)-57)
#define E_BOVR  ((ER)-58)

/* only for ref_mpl() */
#define MPL_STK (-4)
#define MPL_MPL (-3)
#define MPL_SYS (-2)

typedef struct t_ctsk {
    ATR    tskatr;
    VP_INT exinf;
    FP     task;
    PRI    itskpri;
    SIZE   stksz;
    VP     stk;
    char   *name;
} T_CTSK;

typedef struct t_rtsk {
    STAT   tskstat;
    PRI    tskpri;
    PRI    tskbpri;
    STAT   tskwait;
    ID     wobjid;
    TMO    lefttmo;
    UINT   actcnt;
    UINT   wupcnt;
    UINT   suscnt;
} T_RTSK;

typedef struct t_rtst {
    STAT   tskstat;
    STAT   tskwait;
} T_RTST;

typedef struct t_csem {
    ATR    sematr;
    UINT   isemcnt;
    UINT   maxsem;
    char   *name;
} T_CSEM;

typedef struct t_rsem {
    ID     wtskid;
    UINT   semcnt;
} T_RSEM;

typedef struct t_cflg {
    ATR    flgatr;
    FLGPTN iflgptn;
    char   *name;
} T_CFLG;

typedef struct t_rflg {
    ID     wtskid;
    FLGPTN flgptn;
} T_RFLG;

typedef struct t_cdtq {
    ATR    dtqatr;
    UINT   dtqcnt;
    VP     dtq;
    char   *name;
} T_CDTQ;

typedef struct t_rdtq {
    ID     stskid;
    ID     rtskid;
    UINT   sdtqcnt;
} T_RDTQ;

#ifndef MSGS
# define MSGS 16
#endif
typedef struct t_msg {
    struct t_msg *next;
    VB           msgcont[MSGS];
} T_MSG;

typedef struct t_msg_pri {
    struct t_msg_pri *next;
    PRI              msgpri;
    VB               msgcont[MSGS];
} T_MSG_PRI;

typedef struct t_cmbx {
    ATR    mbxatr;
    PRI    maxmpri;
    VP     mprihd;
    char   *name;
} T_CMBX;

typedef struct t_rmbx {
    ID     wtskid;
    T_MSG  *pk_msg;
} T_RMBX;

typedef struct t_cmtx {
    ATR    mtxatr;
    PRI    ceilpri;
    char   *name;
} T_CMTX;

typedef struct t_rmtx {
    ID     htskid;
    ID     wtskid;
} T_RMTX;

typedef struct t_cmbf {
    ATR    mbfatr;
    UINT   maxmsz;
    SIZE   mbfsz;
    VP     mbf;
    char   *name;
} T_CMBF;

typedef struct t_rmbf {
    ID     stskid;
    ID     rtskid;
    UINT   smsgcnt;
    SIZE   fmbfsz;
} T_RMBF;

typedef struct t_cpor {
    ATR    poratr;
    UINT   maxcmsz;
    UINT   maxrmsz;
    char   *name;
} T_CPOR;

typedef struct t_rpor {
    ID     ctskid;
    ID     atskid;
} T_RPOR;

typedef struct t_rrdv {
    ID     wtskid;
} T_RRDV;

typedef struct t_cmpl {
    ATR    mplatr;
    SIZE   mplsz;
    VP     mpl;
    char   *name;
} T_CMPL;

typedef struct t_rmpl {
    ID     wtskid;
    SIZE   fmplsz;
    UINT   fblksz;
} T_RMPL;

typedef struct t_cmpf {
    ATR    mpfatr;
    UINT   blkcnt;
#ifdef _KERNEL_N4
    UINT   blfsz;
#else
    UINT   blksz;
#endif
    VP     mpf;
    char   *name;
} T_CMPF;

typedef struct t_rmpf {
    ID     wtskid;
    UINT   fblkcnt;
} T_RMPF;

typedef struct t_ccyc {
    ATR    cycatr;
    VP_INT exinf;
    FP     cychdr;
    RELTIM cyctim;
    RELTIM cycphs;
    char   *name;
} T_CCYC;

typedef struct t_rcyc {
    STAT   cycstat;
    RELTIM lefttim;
} T_RCYC;

typedef struct t_calm {
    ATR    almatr;
    VP_INT exinf;
    FP     almhdr;
    char   *name;
} T_CALM;

typedef struct t_ralm {
    STAT   almstat;
    RELTIM lefttim;
} T_RALM;

typedef struct t_dovr {
    ATR    ovratr;
    FP     ovrhdr;
    INTNO  intno;
    FP     ovrclr;
    UINT   imask;
} T_DOVR;

typedef struct t_rovr {
    STAT   ovrstat;
    OVRTIM leftotm;
} T_ROVR;

typedef struct t_dtex {
    ATR    texatr;
    FP     texrtn;
} T_DTEX;

typedef struct t_rtex {
    STAT   texstat;
    TEXPTN pndptn;
} T_RTEX;

typedef struct t_dinh {
    ATR    inhatr;
    FP     inthdr;
    UINT   imask;
} T_DINH;

typedef struct t_cisr {
    ATR    isratr;
    VP_INT exinf;
    INTNO  intno;
    FP     isr;
    UINT   imask;
} T_CISR;

typedef struct t_risr {
    INTNO  intno;
    UINT   imask;
} T_RISR;

typedef struct t_dsvc {
    ATR    svcatr;
    FP     svcrtn;
    INT    parn;
} T_DSVC;

typedef struct t_rver {
    UH     maker;
    UH     prid;
    UH     spver;
    UH     prver;
    UH     prno[4];
} T_RVER;

#endif /* _ITRON_H_ */
