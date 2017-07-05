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
#include <stdarg.h>

typedef ER_UINT (*FN0)();
typedef ER_UINT (*FN1)(VP_INT a1);
typedef ER_UINT (*FN2)(VP_INT a1, VP_INT a2);
typedef ER_UINT (*FN3)(VP_INT a1, VP_INT a2, VP_INT a3);
typedef ER_UINT (*FN4)(VP_INT a1, VP_INT a2, VP_INT a3, VP_INT a4);
typedef ER_UINT (*FN5)(VP_INT a1, VP_INT a2, VP_INT a3, VP_INT a4, VP_INT a5);
typedef ER_UINT (*FN6)(VP_INT a1, VP_INT a2, VP_INT a3, VP_INT a4, VP_INT a5, VP_INT a6);

/*
 * extended serivce call hook routine
 */
static ER_UINT svcrtn(FN fncd, ...)
{
    UINT psw;
    ER_UINT r;
    T_SVC *svc;
    va_list v;
    VP_INT a1, a2, a3, a4, a5, a6;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!fncd || _kernel_svcfn_max < (UINT)fncd)
        _KERNEL_RET(E_RSFN);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * call defined function
     */
    if (!(svc = _kernel_svc[fncd - 1]))
        _KERNEL_END(E_RSFN);
    if (!svc->parn) {
        r = (*(FN0)svc->svcrtn)();
    } else {
        va_start(v, fncd);
        switch (svc->parn) {
        case 1 :
            a1 = va_arg(v, VP_INT);
            r = (*(FN1)svc->svcrtn)(a1);
            break;
        case 2 :
            a1 = va_arg(v, VP_INT);
            a2 = va_arg(v, VP_INT);
            r = (*(FN2)svc->svcrtn)(a1, a2);
            break;
        case 3 :
            a1 = va_arg(v, VP_INT);
            a2 = va_arg(v, VP_INT);
            a3 = va_arg(v, VP_INT);
            r = (*(FN3)svc->svcrtn)(a1, a2, a3);
            break;
        case 4 :
            a1 = va_arg(v, VP_INT);
            a2 = va_arg(v, VP_INT);
            a3 = va_arg(v, VP_INT);
            a4 = va_arg(v, VP_INT);
            r = (*(FN4)svc->svcrtn)(a1, a2, a3, a4);
            break;
        case 5 :
            a1 = va_arg(v, VP_INT);
            a2 = va_arg(v, VP_INT);
            a3 = va_arg(v, VP_INT);
            a4 = va_arg(v, VP_INT);
            a5 = va_arg(v, VP_INT);
            r = (*(FN5)svc->svcrtn)(a1, a2, a3, a4, a5);
            break;
        case 6 :
            a1 = va_arg(v, VP_INT);
            a2 = va_arg(v, VP_INT);
            a3 = va_arg(v, VP_INT);
            a4 = va_arg(v, VP_INT);
            a5 = va_arg(v, VP_INT);
            a6 = va_arg(v, VP_INT);
            r = (*(FN6)svc->svcrtn)(a1, a2, a3, a4, a5, a6);
            break;
        default :
            r = E_PAR;
        }
        va_end(v);
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    return r;
}

/*
 * def_svc system call
 */
ER def_svc(FN fncd, const T_DSVC *pk_dsvc)
{
    UINT psw;
    ER r = E_OK;
    T_SVC *svc;
    
    /*
     * init hook
     */
    _KERNEL_SVCHOOK = svcrtn;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!fncd || _kernel_svcfn_max < (UINT)fncd ||
        pk_dsvc && (!pk_dsvc->svcrtn || 6 < pk_dsvc->parn))
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * define
     */
    svc = _kernel_svc[fncd - 1];
    if (pk_dsvc) {
        /*
         * alloc
         */
        if (!svc) {
            if (!(svc = (T_SVC *)_kernel_sys_alloc(sizeof(T_SVC))))
                _KERNEL_END(E_SYS);
            _kernel_svc[fncd - 1] = svc;
        }
        /*
         * init
         */
        svc->svcrtn = pk_dsvc->svcrtn;
        svc->parn   = pk_dsvc->parn;
    }
    /*
     * undefine
     */
    else {
        if (svc) {
            _kernel_sys_free(svc);
            _kernel_svc[fncd - 1] = NULL;
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DEF_SVC();
    return r;
}

/* eof */
