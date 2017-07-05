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

#ifdef _KERNEL_N4
# define blksz blfsz
#endif

ER cre_mpf(ID mpfid, const T_CMPF *pk_cmpf)
{
    UINT psw;
    ER r = E_OK;
    ID id = mpfid;
    T_MPF **tbl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_mpfid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cmpf || !pk_cmpf->blkcnt || !pk_cmpf->blksz ||
        (unsigned)pk_cmpf->mpf & TMSK_ALIGN)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * alloc auto id
     */
    if (id == ID_ANY) {
        id = _kernel_mpfid_max;
        tbl = &_kernel_mpf[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_mpf[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init MPF
     */
    {
        /*
         * alloc MPF struct
         */
        T_MPF *mpf;
        SIZE siz = sizeof(T_MPF);
        PRI pri = 1;
        if (pk_cmpf->mpfatr & TA_TPRI) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if ((char *)pk_cmpf < _KERNEL_ETEXT()) {
            if (!(mpf = (T_MPF *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            mpf->cmpf = (T_CMPF *)pk_cmpf;
        } else {
            if (!(mpf = (T_MPF *)_kernel_sys_alloc(siz + sizeof(T_CMPF))))
                _KERNEL_END(E_SYS);
            mpf->cmpf = (T_CMPF *)((char *)mpf + siz);
            *mpf->cmpf = *pk_cmpf;
        }
        /*
         * alloc memory block
         */
        {
            SIZE siz = TSZ_MPF(pk_cmpf->blkcnt, pk_cmpf->blksz);
            char *str;
            if (pk_cmpf->mpf) {
                str = pk_cmpf->mpf;
            } else {
                if (!(str = (char *)_kernel_mpl_alloc(siz))) {
                    _kernel_sys_free(mpf);
                    _KERNEL_END(E_NOMEM);
                }
            }
            mpf->str = str;
            mpf->end = str + siz;
            *tbl = mpf;
            /*
             * init
             */
            _kernel_nul_msg_que(&mpf->free, 1);
            {
                UINT cnt = mpf->cnt = pk_cmpf->blkcnt;
                SIZE bsz = TSZ_MPFBLK(pk_cmpf->blksz);
                for (; cnt--; str += bsz) {
                    ((T_MSG *)str)->next = NULL;
                    _kernel_enq_msg(&mpf->free, (T_MSG *)str, 1);
                }
            }
            _kernel_nul_que(&mpf->que[0], pri);
        }
    }
    /*
     * acre_??? will return id
     */
    if (mpfid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_MPF();
    return r;
}

/* eof */
