/*
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

/*
 * Get max of kernel memory
 */
static void _kernel_maxmem(T_KERBLK_HEAD *head, SIZE *p_fmplsz, UINT *p_fblksz)
{
    T_KERBLK *zero, *blk;
    SIZE fmplsz = 0;
    SIZE fblksz = 0;

    zero = &head->zero;
    for (blk = zero->next; blk != zero; blk = blk->next) {
        SIZE len = blk->len;
        if (len)
            len -= sizeof(T_KERBLK);
        fmplsz += len;
        if (fblksz < len)
            fblksz = len;
    }
    *p_fmplsz = fmplsz;
#if _KERNEL_INTSIZ < 4
    if (fblksz > TMAX_UINT)
        fblksz = TMAX_UINT;
#endif
    *p_fblksz = (UINT)fblksz;
}

/*
 * ref_mpl system call
 *   (special case)
 *   id = -4 : kernel stack memory info
 *   id = -3 : kernel system memory info
 *   id = -2 : kernel memory-pool memory info
 */
ER ref_mpl(ID id, T_RMPL *pk_rmpl)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    T_MPL *mpl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!(-4 <= id && id <= -2 || 1 <= id && (UINT)id < _kernel_mplid_max))
        _KERNEL_RET(E_ID);
    if (!pk_rmpl)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get info
     */
    if (0 < id) {
        if (!(mpl = _kernel_mpl[id - 1]))
            _KERNEL_END(E_NOEXS);
        tcb = (T_TCB *)_kernel_first(&mpl->que[0],
                                     (mpl->cmpl->mplatr & TA_TPRI)?
                                     _kernel_tpri_max : 1);
        pk_rmpl->wtskid = (tcb)? tcb->id : TSK_NONE;
        _kernel_maxmem(&mpl->head, &pk_rmpl->fmplsz, &pk_rmpl->fblksz);
    } else {
        T_KERBLK_HEAD *head;
        if (id == MPL_STK) {
            head = &_kernel_stkmem;
        } else if (id == MPL_MPL) {
            head = _kernel_mplmem;
        } else { /* id == MPL_SYS */
            head = _kernel_sysmem;
        }
        pk_rmpl->wtskid = TSK_NONE;
        _kernel_maxmem(head, &pk_rmpl->fmplsz, &pk_rmpl->fblksz);
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_REF_MPL();
    return r;
}

/* eof */
