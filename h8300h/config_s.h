#ifndef _CONFIG_S_H_
#define _CONFIG_S_H_
/*
 * config_s.h : Sub definition of config.h for H8/300H
 *
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

#ifndef ISTKSZ
# define ISTKSZ 512
#endif

#ifndef TSTKSZ
# define TSTKSZ 512
#endif

#ifndef KNL_LEVEL
# define KNL_LEVEL 1             /* 1 or 2 */
#endif
#define KNL_LEVEL_MAX 2

/*
 * interrupt mask value of kernel level
 */
#if KNL_LEVEL == 2
UINT _kernel_msk = 0xc000; /* CCR I=1 UI=1 */
#elif KNL_LEVEL == 1
UINT _kernel_msk = 0x8000; /* CCR I=1 UI=0 */
#else
# error bad KNL_LEVEL
#endif

#endif /* _CONFIG_S_H_ */
