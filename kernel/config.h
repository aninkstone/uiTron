#ifndef _CONFIG_H_
#define _CONFIG_H_
/*
 * config.h : Define kernel configuration
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

/****************************************
 *      kernel configuration symbols    *
 ****************************************/

/* maximum number of task priority : 0..255 */
#ifndef TPRI_MAX
# define TPRI_MAX       8
#endif

/* maximum number of tasks : 0..255 */
#ifndef TSKID_MAX
# define TSKID_MAX      8
#endif

/* maximum number of semaphores : 0..TMAX_INT */
#ifndef SEMID_MAX
# define SEMID_MAX      8
#endif

/* maximum number of eventflags : 0..TMAX_INT */
#ifndef FLGID_MAX
# define FLGID_MAX      8
#endif

/* maximum number of data queues : 0..TMAX_INT */
#ifndef DTQID_MAX
# define DTQID_MAX      8
#endif

/* maximum number of mailboxes : 0..TMAX_INT */
#ifndef MBXID_MAX
# define MBXID_MAX      8
#endif

/* maximum number of mutexes : 0..TMAX_INT */
#ifndef MTXID_MAX
# define MTXID_MAX      8
#endif

/* maximum number of message buffers : 0..TMAX_INT */
#ifndef MBFID_MAX
# define MBFID_MAX      8
#endif

/* maximum number of randezvous : 0..TMAX_INT */
#ifndef PORID_MAX
# define PORID_MAX      8
#endif

/* maximum number of interrupt serivce routines : 0..TMAX_INT */
#ifndef ISRID_MAX
# define ISRID_MAX      8
#endif

/* maximum number of variable-sized memory pools : 0..TMAX_INT */
#ifndef MPLID_MAX
# define MPLID_MAX      8
#endif

/* maximum number of fixed-sized memory pools : 0..TMAX_INT */
#ifndef MPFID_MAX
# define MPFID_MAX      8
#endif

/* maximum number of cyclic handlers : 0..TMAX_INT */
#ifndef CYCNO_MAX
# define CYCNO_MAX      8
#endif

/* maximum number of alarm handlers : 0..TMAX_INT */
#ifndef ALMNO_MAX
# define ALMNO_MAX      8
#endif

/* maximum number of extended service calls : 0..TMAX_INT */
#ifndef SVCFN_MAX
# define SVCFN_MAX      8
#endif

/* task timer hashed queue size : 1,2,4,8, ... 256 */
#ifndef TMRQSZ
# define TMRQSZ         8
#endif
#if TMRQSZ != 1 && TMRQSZ != 2 && TMRQSZ != 4 && TMRQSZ != 8 && TMRQSZ != 16 && TMRQSZ != 32 && TMRQSZ != 64 && TMRQSZ != 128 && TMRQSZ != 256
# error : Bad TMRQSZ
#endif

/* cyclic handler timer hashed queue size : 1,2,4,8, ... 256 */
#ifndef CYCQSZ
# define CYCQSZ         8
#endif
#if CYCQSZ != 1 && CYCQSZ != 2 && CYCQSZ != 4 && CYCQSZ != 8 && CYCQSZ != 16 && CYCQSZ != 32 && CYCQSZ != 64 && CYCQSZ != 128 && CYCQSZ != 256
# error : Bad CYCQSZ
#endif

/* alarm handler timer hashed queue size : 1,2,4,8, ... 256 */
#ifndef ALMQSZ
# define ALMQSZ         8
#endif
#if ALMQSZ != 1 && ALMQSZ != 2 && ALMQSZ != 4 && ALMQSZ != 8 && ALMQSZ != 16 && ALMQSZ != 32 && ALMQSZ != 64 && ALMQSZ != 128 && ALMQSZ != 256
# error : Bad ALMQSZ
#endif

/* system memory size : 0 = use stack memory */
#ifndef SYSMSZ
# define SYSMSZ 0
#endif

/* memory pool (mpf/mpl/mbf) memory size : 0 = use stack memory */
#ifndef MPLMSZ
# define MPLMSZ 0
#endif

/* stack memory size : 0 = use main() stack */
#ifndef STKMSZ
# define STKMSZ 0
#endif

/* ISR hashed queue size : 1,2,4,8, ... 65536 */
#ifndef ISRQSZ
# define ISRQSZ         8
#endif
#if ISRQSZ != 1 && ISRQSZ != 2 && ISRQSZ != 4 && ISRQSZ != 8 && ISRQSZ != 16 && ISRQSZ != 32 && ISRQSZ != 64 && ISRQSZ != 128 && ISRQSZ != 0x100 && ISRQSZ != 0x200 && ISRQSZ != 0x400 && ISRQSZ != 0x800 && ISRQSZ != 0x1000 && ISRQSZ != 0x2000 && ISRQSZ != 0x4000 && ISRQSZ != 0x8000 && ISRQSZ != 0x10000
# error : Bad ISRQSZ
#endif

/********************************
 *      kernel varialbes        *
 ********************************/

T_LNK _kernel_rdq[TPRI_MAX + 1];
PRI _kernel_tpri_max _KERNEL_DATA = TPRI_MAX;

T_TCB *_kernel_tcb[TSKID_MAX];
UINT _kernel_tskid_max  _KERNEL_DATA = TSKID_MAX;

T_SEM *_kernel_sem[SEMID_MAX];
UINT _kernel_semid_max  _KERNEL_DATA = SEMID_MAX;

T_FLG *_kernel_flg[FLGID_MAX];
UINT _kernel_flgid_max  _KERNEL_DATA = FLGID_MAX;

T_DTQ *_kernel_dtq[DTQID_MAX];
UINT _kernel_dtqid_max  _KERNEL_DATA = DTQID_MAX;

T_MBX *_kernel_mbx[MBXID_MAX];
UINT _kernel_mbxid_max  _KERNEL_DATA = MBXID_MAX;

T_MTX *_kernel_mtx[MTXID_MAX];
UINT _kernel_mtxid_max  _KERNEL_DATA = MTXID_MAX;

T_MBF *_kernel_mbf[MBFID_MAX];
UINT _kernel_mbfid_max  _KERNEL_DATA = MBFID_MAX;

T_POR *_kernel_por[PORID_MAX];
UINT _kernel_porid_max  _KERNEL_DATA = PORID_MAX;

T_ISR *_kernel_isr[ISRID_MAX];
UINT _kernel_isrid_max  _KERNEL_DATA = ISRID_MAX;

T_MPL *_kernel_mpl[MPLID_MAX];
UINT _kernel_mplid_max  _KERNEL_DATA = MPLID_MAX;

T_MPF *_kernel_mpf[MPFID_MAX];
UINT _kernel_mpfid_max  _KERNEL_DATA = MPFID_MAX;

T_CYC *_kernel_cyc[CYCNO_MAX];
UINT _kernel_cycno_max  _KERNEL_DATA = CYCNO_MAX;

T_ALM *_kernel_alm[ALMNO_MAX];
UINT _kernel_almno_max  _KERNEL_DATA = ALMNO_MAX;

T_SVC *_kernel_svc[SVCFN_MAX];
UINT _kernel_svcfn_max  _KERNEL_DATA = SVCFN_MAX;

T_LNK _kernel_tmrq_array[TMRQSZ];
T_TIMQUE _kernel_tmrq = {TMRQSZ - 1, 0, &_kernel_tmrq_array[0]};

T_LNK _kernel_cycq_array[CYCQSZ];
T_TIMQUE _kernel_cycq = {CYCQSZ - 1, 0, &_kernel_cycq_array[0]};

T_LNK _kernel_almq_array[ALMQSZ];
T_TIMQUE _kernel_almq = {ALMQSZ - 1, 0, &_kernel_almq_array[0]};

SIZE _kernel_sysmsz _KERNEL_DATA = TSZ_PTR(SYSMSZ);
SIZE _kernel_mplmsz _KERNEL_DATA = TSZ_PTR(MPLMSZ);
SIZE _kernel_stkmsz _KERNEL_DATA = TSZ_PTR(STKMSZ);

T_LNK _kernel_isrq[ISRQSZ];
UH _kernel_isrq_msk _KERNEL_DATA = ISRQSZ - 1;

#include "config_s.h"

/* interrupt handler stack size */
#if ISTKSZ == 0
# error : ISTKSZ must not to be 0
#endif
SIZE _kernel_istksz _KERNEL_DATA = TSZ_PTR(ISTKSZ);

/* time event handler stack size */
#if TSTKSZ == 0
# error : TSTKSZ must not to be 0
#endif
SIZE _kernel_tstksz _KERNEL_DATA = TSZ_PTR(TSTKSZ);

#if KNL_LEVEL == 0
# error : KNL_LEVEL must not to be 0
#endif
UINT _kernel_level = KNL_LEVEL; /* interrupt level at critical section */

#if KNL_LEVEL_MAX == 0
# error : KNL_LEVEL_MAX must not to be 0
#endif
UINT _kernel_level_max = KNL_LEVEL_MAX; /* maximum interrupt level */

#endif /* _CONFIG_H_ */
