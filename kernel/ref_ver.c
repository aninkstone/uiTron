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

const struct {
    char s0;
    char s1;
    char s2;
    char s3;
    char s4;
    char s5;
    char s6;
    char s7;
    T_RVER rver;
} _kernel_ver = {
    'v', 'e', 'r', 's', 'i', 'o', 'n', '\0',
    {
        TKERNEL_MAKER,
        TKERNEL_PRID,
        TKERNEL_SPVER,
        TKERNEL_PRVER,
        {0, 0, 0, 0}
    }
};

/*
 * ref_ver system call
 */
ER ref_ver(T_RVER *pk_rver)
{
    ER r = E_OK;
    
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!pk_rver)
        _KERNEL_RET(E_PAR);
    /*
     * get version info
     */
    *pk_rver = _kernel_ver.rver;
 ret:
    _KERNEL_ASSERT_REF_VER();
    return r;
}

/* end */
