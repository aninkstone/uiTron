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
 * cre_mbf/acre_mbf system call
 */
ER cre_mbf(ID mbfid, const T_CMBF *pk_cmbf)
{
    UINT psw;
    ER r = E_OK;
    ID id = mbfid;
    T_MBF **tbl;
    
    /*
     * init hook
     */
    _kernel_chk_wmbf_fp = _kernel_chk_wmbf;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_mbfid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cmbf || !pk_cmbf->maxmsz ||
        (unsigned int)pk_cmbf->mbf & TMSK_ALIGN)
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
        id = _kernel_mbfid_max;
        tbl = &_kernel_mbf[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_mbf[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init MBF
     */
    {
        /*
         * alloc MBF struct
         */
        T_MBF *mbf;
        SIZE siz = sizeof(T_MBF);
        PRI spri = 1;
        PRI rpri = 1;
        if (pk_cmbf->mbfatr & TA_TPRI) {
            spri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (spri - 1);
        }
        if (pk_cmbf->mbfatr & TA_TPRIR) {
            rpri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * rpri;
        }
        if ((char *)pk_cmbf < _KERNEL_ETEXT()) {
            if (!(mbf = (T_MBF *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            mbf->cmbf = (T_CMBF *)pk_cmbf;
        } else {
            if (!(mbf = (T_MBF *)_kernel_sys_alloc(siz + sizeof(T_CMBF))))
                _KERNEL_END(E_SYS);
            mbf->cmbf = (T_CMBF *)((char *)mbf + siz);
            *mbf->cmbf = *pk_cmbf;
        }
        /*
         * alloc ring buffer
         */
        if (pk_cmbf->mbfsz && !pk_cmbf->mbf) {
            if (!(mbf->buf = (UB *)_kernel_mpl_alloc(pk_cmbf->mbfsz))) {
                _kernel_sys_free(mbf);
                _KERNEL_END(E_NOMEM);
            }
        } else {
                mbf->buf = pk_cmbf->mbf;
        }
        *tbl = mbf;
        /*
         * init
         */
        mbf->out = mbf->in = mbf->buf;
        mbf->sz = 0;
        mbf->smsgcnt = 0;
        _kernel_nul_que(&mbf->sque[0], spri);
        _kernel_nul_que(&mbf->sque[spri], rpri); /* &mbf->rque[0] */
    }
    /*
     * acre_??? will return id
     */
    if (mbfid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_MBF();
    return r;
}

/* eof */
