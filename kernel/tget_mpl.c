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
 * get_mpl/pget_mpl/tget_mpl system call
 */
ER tget_mpl(ID id, UINT blksz, VP *p_blk, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_MPL *mpl;
    UB s = _kernel_sts;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (tmout == TMO_POL) {
        if (_KERNEL_CHK_IL(s))
            _KERNEL_RET(E_CTX);
    } else {
        if (_KERNEL_CHK_WAI(s, tmout))
            _KERNEL_RET(E_CTX);
    }
    /*
     * check par
     */
    if (!id || _kernel_mplid_max < (UINT)id)
        _KERNEL_RET(E_ID);
    if (!blksz || !p_blk)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get memory block
     */
    if (!(mpl = _kernel_mpl[id - 1]))
        _KERNEL_END(E_NOEXS);
    if (_KERNEL_CHK_IS(s)) {
        char *blk = _kernel_alloc(&mpl->head, blksz);
        if (!blk)
            _KERNEL_END_NOLOG(E_TMOUT);
        *p_blk = (VP)blk;
    } else {                    /* task context */
        T_TCB *tcb = (T_TCB *)_kernel_cur;
        PRI pri = (mpl->cmpl->mplatr & TA_TPRI)? tcb->pri : 1;
        if (!_kernel_first(&mpl->que[0], pri)) {
            char *blk = _kernel_alloc(&mpl->head, blksz);
            if (blk) {
                *p_blk = (VP)blk;
                goto end;
            }
        }
        /*
         * get-memory-block waiting
         */
        if (tmout == TMO_POL)
            _KERNEL_END_NOLOG(E_TMOUT);
        tcb->wai = TTW_MPL;
        tcb->wid = id;
        _KERNEL_SET_REGPAR_DAT(p_blk);
        _KERNEL_SET_REGPAR_SIZ(blksz);
        _KERNEL_SET_REGPAR_FLG();
        r = _kernel_wai(tcb, tmout, &mpl->que[0],
                        mpl->cmpl->mplatr & TA_TPRI); /* dispatch */
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TGET_MPL();
    return r;
}

/* eof */
