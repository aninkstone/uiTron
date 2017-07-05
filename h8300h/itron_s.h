#ifndef _ITRON_S_H_
#define _ITRON_S_H_
/*
 * itron_s.h : Sub definition of itron.h for H8/300H
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

/* ITRON data */
typedef signed char        B;
typedef short              H;
typedef long               W;
typedef long long          D;
typedef unsigned char      UB;
typedef unsigned short     UH;
typedef unsigned long      UW;
typedef unsigned long long UD;
typedef signed char        VB;
typedef short              VH;
typedef long               VW;
typedef long long          VD;
typedef void *             VP;
typedef void               (*FP)();
typedef short              INT;
typedef unsigned short     UINT;
typedef int                BOOL;
typedef short              FN;
typedef short              ER;
typedef short              ID;
typedef unsigned short     ATR;
typedef unsigned short     STAT;
typedef unsigned short     MODE;
typedef short              PRI;
typedef unsigned long      SIZE;
typedef unsigned long      TMO;
typedef unsigned long      RELTIM;
typedef unsigned long      OVRTIM;
typedef struct t_systim {
    H utime;
    UW ltime;
} SYSTIM;
typedef void *             VP_INT;
typedef int                ER_BOOL;
typedef short              ER_ID;
typedef short              ER_UINT;
typedef unsigned short     TEXPTN;
typedef unsigned short     FLGPTN;
typedef unsigned short     RDVPTN;
typedef short              INHNO;
typedef short              INTNO;
typedef short              EXCNO;
typedef short              RDVNO;

/* max value */
#define TMAX_INT           0x7fff
#define TMAX_UINT          0xffff

/* sizeof(INT)/sizeof(UINT) */
#define _KERNEL_INTSIZ 2

#endif /* _ITRON_S_H_ */
