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
 * dis_tex system call
 */
ER dis_tex()
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    UB s = _kernel_sts;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(s))
        _KERNEL_RET(E_CTX);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    if (!(tcb = (T_TCB *)_kernel_cur))
        _KERNEL_END_NOLOG(E_OK); /* interrupt case */
    /*
     * disable
     */
    {
        T_TEX *tex = (T_TEX *)tcb->tex;  /* avoid volatile effect */
        if (!tex) {
            if (_KERNEL_CHK_TSK(s)) {
                _KERNEL_END(E_OBJ);
            } else {
                _KERNEL_END_NOLOG(E_OK);
            }
        }
        tex->stat = TTEX_DIS;
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DIS_TEX();
    return r;
}

/* end */
