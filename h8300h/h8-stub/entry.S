#include "linkage.h"

/* SELECT SIO CH (0/1/2) */
#ifndef CH
# define CH 1
#endif

        .h8300h
        .file "entry.S"

        .section .text
        .align 2
        .global _start

P1DDR = 0xfee000
P2DDR = 0xfee001
P8DDR = 0xfee007
IPRB = 0xfee019
DRCRA = 0xfee026
DRCRB = 0xfee027
RTMCSR = 0xfee028
RTCOR = 0xfee02a
;----------------------------------------------------------------------------
ENTRY(start)
;
/* Initialize Stack Pointer */

        mov.l   #SYMBOL_NAME(stub_stack)+0x500, er7
        
/* HW Initialize */

; P1 is address bus(A0-7)
        mov.b   #0xff, r0l
        mov.b   r0l, @P1DDR
        
; P2 is address bus(A8-10)
        mov.b   #0x07, r0l
        mov.b   r0l, @P2DDR
        
; P80, CS1-2 enable
        mov.b   #0x0c, r0l
        mov.b   r0l, @P8DDR

; RTCOR 20MHz - 32ms refresh / 2048 raw (15.6us)
        mov.b   #39, r0l
        mov.b   r0l, @RTCOR

; RTMCSR CLK / 8
        mov.b   #0x10, r0l
        mov.b   r0l, @RTMCSR

; DRCRB = MXC1 | RCYCE
        mov.b   #0x90, r0l
        mov.b   r0l, @DRCRB

; DRCRA = DRAS0 
        mov.b   #0x20, r0l
        mov.b   r0l, @DRCRA

; DRAM Initial Wait
        mov.l   #400000, er0
L1:
        subs    #1, er0
        cmp.l   #0, er0
        bne     L1
        
; Initialize Serial Port
        jsr     SYMBOL_NAME(init_serial)

/* Initialize BSS */

        mov.l   #__bss_start, er0
        mov.l   #__bss_end, er1
        sub.l   er2, er2
L2:
        mov.l   er2, @er0
        add.l   #4, er0
        cmp.l   er1, er0
        blo     L2
        
/* Initialize Data */

        mov.l   #__idata_start, er0
        mov.l   #__idata_end, er1
        mov.l   #__data_start, er2
L3:
        mov.l   @er0+, er4
        mov.l   er4, @er2
        add.l   #4, er2
        cmp.l   er1, er0
        blo     L3

/* Jump to GDB_STUB */

        jmp             SYMBOL_NAME(start_gdbstub)
