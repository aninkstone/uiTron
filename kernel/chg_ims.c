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
 * chg_ims system call
 *   imask : interrupt mask level (0,1,2 ... N)
 */
ER chg_ims(UINT imask)
{
    UINT psw;
    ER r = E_OK;

    /*
     * start critical section
     */
    _KERNEL_DIS();
    /*
     * level 0 : unlock CPU and enable dispatch
     */
    if (!imask) {
        _kernel_sts &= ~(T_KER_LOC | T_KER_DSP);
        if (_kernel_dly)
            _kernel_highest(); /* dispatch */
    }
    /*
     * 0 < level < KNL_LEVEL : unlock CPU and disable dispatch
     * KNL_LEVEL <= level    : lock   CPU and disable dispatch
     */
    else {
        UB sts = _kernel_sts;
        if (imask < _kernel_level) {
            sts &= ~T_KER_LOC;
        } else {
            sts |= T_KER_LOC;
        }
        _kernel_sts = sts | T_KER_DSP;
    }
    /*
     * set interrupt mask level
     */
    psw = _KERNEL_LVL2PSW(imask);
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CHG_IMS();
    return r;
}

/* end */
