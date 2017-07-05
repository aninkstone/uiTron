/*
 * ram.x : linker script for AE-H8/3069
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
OUTPUT_FORMAT("elf32-h8300")
OUTPUT_ARCH(h8300h)
ENTRY("_start")

MEMORY
{
    RAM(wx) : ORIGIN = 0x400000, LENGTH = 0x200000
    STK(r)  : ORIGIN = 0x600000, LENGTH = 4
}

SECTIONS
{ 
    .text : {
        *(.text)
        *(.strings)
        *(.rodata)
        _etext = . ; 
    } > RAM

    .data : {
        . = ALIGN(4) ;
        _data = . ; 
        *(.data)
        . = ALIGN(4) ;
        _edata = . ; 
    } > RAM

    .bss : {
        . = ALIGN(4) ;
        _bss = . ;  
        *(.bss)
        *(COMMON)
        . = ALIGN(4) ;
        _end = . ;  
    } > RAM

    .stack : {
        _stack = . ;
    } > STK
}
