/*
 * sio.h: The header of interrupt driven serial driver
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

#ifndef TA_BYTE
# error : This driver requires byte mode data queue.
#endif

#define SIO_7BIT 0x40
#define SIO_PTY  0x20
#define SIO_ODD  0x10
#define SIO_2STP 0x08

void put_sio(int ch, int data);
int get_sio(int ch, TMO tmout);
ER ini_sio(int ch, int bps, int option, UINT bufsiz);

/* end */
