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
 * cre_dtq/acre_dtq system call
 */
ER cre_dtq(ID dtqid, const T_CDTQ *pk_cdtq)
{
    UINT psw;
    ER r = E_OK;
    ID id = dtqid;
    T_DTQ **tbl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_dtqid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cdtq ||
        !(pk_cdtq->dtqatr & TA_BYTE) &&
        (unsigned int)pk_cdtq->dtq & TMSK_ALIGN)
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
        id = _kernel_dtqid_max;
        tbl = &_kernel_dtq[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_dtq[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init DTQ
     */
    {
        /*
         * alloc DTQ struct and ring buffer
         */
        T_DTQ *dtq;
        SIZE siz = sizeof(T_DTQ);
        PRI pri = 1;
        B buf = 0;
        if (pk_cdtq->dtqatr & TA_TPRI) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if (pk_cdtq->dtqcnt && !pk_cdtq->dtq) {
            buf = 1;
            siz += (pk_cdtq->dtqatr & TA_BYTE)?
                TSZ_DTB(pk_cdtq->dtqcnt) : TSZ_DTQ(pk_cdtq->dtqcnt);
        }
        if ((char *)pk_cdtq < _KERNEL_ETEXT()) {
            if (!(dtq = (T_DTQ *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            dtq->cdtq = (T_CDTQ *)pk_cdtq;
        } else {
            if (!(dtq = (T_DTQ *)_kernel_sys_alloc(siz + sizeof(T_CDTQ))))
                _KERNEL_END(E_SYS);
            dtq->cdtq = (T_CDTQ *)((char *)dtq + siz);
            *dtq->cdtq = *pk_cdtq;
        }
        dtq->buf = (buf)? (VP_INT *)&dtq->sque[pri] : (VP_INT *)pk_cdtq->dtq;
        *tbl = dtq;
        /*
         * init
         */
        dtq->ip = dtq->op = dtq->cnt = 0;
        _kernel_nul_que(&dtq->rque, 1);
        _kernel_nul_que(&dtq->sque[0], pri);
    }
    /*
     * acre_??? will return id
     */
    if (dtqid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_DTQ();
    return r;
}

/* eof */
