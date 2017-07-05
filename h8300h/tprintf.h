#ifndef TPRINTF_H
#define TPRINTF_H
/*
 * tprintf.h : Define tiny printf functions
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

#define printf  t_printf
#define fprintf t_fprintf
#define vprintf t_vprintf

int t_printf(const char *f, ...);
int t_fprintf(const int ch, const char *f, ...);
int t_sprintf(char *s, const char *f, ...);
void _kernel_panic(char *file, int line);
void _kernel_assert(char *f, ...);

#endif /* TPRINTF_H */
