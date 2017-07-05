//
// ramcrt0.S : C start up routine to debug on DRAM by GDB for AE-H8/3069
//
// Copyright (C) 2007,2008,2009,2010
// 256TECH Co., Ltd.
// Masahiro Sakamoto (m-sakamoto@users.sourceforge.net)
//
// This file is part of URIBO.
//
// URIBO is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// URIBO is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with URIBO; see the file COPYING and COPYING.LESSER.
// If not, see <http://www.gnu.org/licenses/>.
//

        .h8300h
        .section .text
        .global _start
        .extern _main
        .extern _bss
        .extern _end

// set SP to end of DRAM
_start:
        mov.l   #_stack, sp

// clear BSS
        mov.l   #_bss, er0
        mov.l   #_end, er1
        sub.l   er2, er2
bss_init_loop:
        cmp     er1, er0
        bhs     bss_init_end
        mov.l   er2, @er0
        adds    #4, er0
        bra     bss_init_loop
bss_init_end:

// call main
        jsr     @_main

// return to gdb
_exit:
        rts
// end
