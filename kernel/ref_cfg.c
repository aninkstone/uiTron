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

static const T_RCFG rcvg =
{
};

/*
 * ref_cfg system call
 */
ER ref_cfg(T_RCFG *pk_rcfg)
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
    if (!pk_rcfg)
        _KERNEL_RET(E_PAR);
    /*
     * get kernel config info
     */
    pk_rcfg->tskid_max = _kernel_tskid_max;
    pk_rcfg->semid_max = _kernel_semid_max;
    pk_rcfg->flgid_max = _kernel_flgid_max;
    pk_rcfg->mbxid_max = _kernel_mbxid_max;
    pk_rcfg->mbfid_max = _kernel_mbfid_max;
    pk_rcfg->porid_max = _kernel_porid_max;
    pk_rcfg->mplid_max = _kernel_mplid_max;
    pk_rcfg->mpfid_max = _kernel_mpfid_max;
    pk_rcfg->cycno_max = _kernel_cycno_max;
    pk_rcfg->almno_max = _kernel_almno_max;
    pk_rcfg->tpri_max  = _kernel_tpri_max;
    pk_rcfg->tmrqsz = (int)((unsigned int)_kernel_tmrq.msk + 1);
    pk_rcfg->cycqsz = (int)((unsigned int)_kernel_cycq.msk + 1);
    pk_rcfg->almqsz = (int)((unsigned int)_kernel_almq.msk + 1);
    pk_rcfg->istksz = (int)_kernel_istksz;
    pk_rcfg->tstksz = (int)_kernel_tstksz;
    pk_rcfg->sysmsz = _kernel_sysmsz;
    pk_rcfg->mplmsz = _kernel_mplmsz;
    pk_rcfg->stkmsz = _kernel_stkmsz;
    pk_rcfg->dtqid_max = _kernel_dtqid_max;
    pk_rcfg->mtxid_max = _kernel_mtxid_max;
    pk_rcfg->isrid_max = _kernel_isrid_max;
    pk_rcfg->svcfn_max = _kernel_svcfn_max;
    pk_rcfg->isrqsz = (int)((unsigned int)_kernel_isrq_msk + 1);
ret:
    _KERNEL_ASSERT_REF_CFG();
    return r;
}

/* end */
