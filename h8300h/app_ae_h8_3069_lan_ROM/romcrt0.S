//
// romcrt0.S : C start up routine to run on ROM for AE-H8/3069
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
        .file "romcrt0.S"
        .h8300h

//
// definitions
//
        .extern __kernel_int_entry
        .extern _idata
        .extern _data
        .extern _edata
        .extern _bss
        .extern _end
        .extern _ini_sci
        .extern _main

        .macro os_entry intid
        push.l  er0
        push.l  er1
        mov     #\intid, er0
        jmp     __kernel_int_entry
        .endm

//H8/3069F
        .equ    IntID_start     ,0
        .equ    IntID_Reserved  ,1
        .equ    IntID_NMI       ,7
        .equ    IntID_TRAP0     ,8
        .equ    IntID_TRAP1     ,9
        .equ    IntID_TRAP2     ,10
        .equ    IntID_TRAP3     ,11
        .equ    IntID_IRQ0      ,12
        .equ    IntID_IRQ1      ,13
        .equ    IntID_IRQ2      ,14
        .equ    IntID_IRQ3      ,15
        .equ    IntID_IRQ4      ,16
        .equ    IntID_IRQ5      ,17
        .equ    IntID_WOVI      ,20
        .equ    IntID_CMI       ,21
        .equ    IntID_ADI       ,23
        .equ    IntID_IMIA0     ,24
        .equ    IntID_IMIB0     ,25
        .equ    IntID_OVI0      ,26
        .equ    IntID_IMIA1     ,28
        .equ    IntID_IMIB1     ,29
        .equ    IntID_OVI1      ,30
        .equ    IntID_IMIA2     ,32
        .equ    IntID_IMIB2     ,33
        .equ    IntID_OVI2      ,34
        .equ    IntID_CMIA0     ,36
        .equ    IntID_CMIB0     ,37
        .equ    IntID_CMIA1B1   ,38
        .equ    IntID_TOVI01    ,39
        .equ    IntID_CMIA2     ,40
        .equ    IntID_CMIB2     ,41
        .equ    IntID_CMIA3B3   ,42
        .equ    IntID_TOVI23    ,43
        .equ    IntID_DEND0A    ,44
        .equ    IntID_DEND0B    ,45
        .equ    IntID_DEND1A    ,46
        .equ    IntID_DEND1B    ,47
        .equ    IntID_ERI0      ,52
        .equ    IntID_RXI0      ,53
        .equ    IntID_TXI0      ,54
        .equ    IntID_TEI0      ,55
        .equ    IntID_ERI1      ,56
        .equ    IntID_RXI1      ,57
        .equ    IntID_TXI1      ,58
        .equ    IntID_TEI1      ,59
        .equ    IntID_ERI2      ,60
        .equ    IntID_RXI2      ,61
        .equ    IntID_TXI2      ,62
        .equ    IntID_TEI2      ,63

        .equ    P1DDR           ,0xfee000
        .equ    P2DDR           ,0xfee001
        .equ    P8DDR           ,0xfee007
        .equ    IPRB            ,0xfee019
        .equ    DRCRA           ,0xfee026
        .equ    DRCRB           ,0xfee027
        .equ    RTMCSR          ,0xfee028
        .equ    RTCOR           ,0xfee02a

//
// Interrupt vecotrs x64
//
        .section .text
        .balign 4

        .data.l _start          //00 Power On Reset
        .data.l INT_Reserved    //01 Reserved
        .data.l INT_Reserved    //02 Reserved
        .data.l INT_Reserved    //03 Reserved
        .data.l INT_Reserved    //04 Reserved
        .data.l INT_Reserved    //05 Reserved
        .data.l INT_Reserved    //06 Reserved
        .data.l INT_NMI         //07 NMI
        .data.l INT_TRAP0       //08 User Breakpoint Trap
        .data.l INT_TRAP1       //09 TRAP1
        .data.l INT_TRAP2       //10 TRAP2
        .data.l INT_TRAP3       //11 TRAP3
        .data.l INT_IRQ0        //12 IRQ0
        .data.l INT_IRQ1        //13 IRQ1
        .data.l INT_IRQ2        //14 IRQ2
        .data.l INT_IRQ3        //15 IRQ3
        .data.l INT_IRQ4        //16 IRQ4
        .data.l INT_IRQ5        //17 IRQ5
        .data.l INT_Reserved    //18 Reserved
        .data.l INT_Reserved    //19 Reserved
        .data.l INT_WOVI        //20 WOVI (WatchFog)
        .data.l INT_CMI         //21 CMI (DRAM)
        .data.l INT_Reserved    //22 Reserved
        .data.l INT_ADI         //23 ADI (A/D)
        .data.l INT_IMIA0       //24 IMIA0 (16bit Timer ch0)
        .data.l INT_IMIB0       //25 IMIB0 (16bit Timer ch0)
        .data.l INT_OVI0        //26 OVI0 (16bit Timer ch0)
        .data.l INT_Reserved    //27 Reserved
        .data.l INT_IMIA1       //28 IMIA1 (16bit Timer 1)
        .data.l INT_IMIB1       //29 IMIB1 (16bit Timer 1)
        .data.l INT_OVI1        //30 OVI1 (16bit Timer 1)
        .data.l INT_Reserved    //31 Reserved
        .data.l INT_IMIA2       //32 IMIA2 (16bit Timer 2)
        .data.l INT_IMIB2       //33 IMIB2 (16bit Timer 2)
        .data.l INT_OVI2        //34 OVI2 (16bit Timer 2)
        .data.l INT_Reserved    //35 Reserved
        .data.l INT_CMIA0       //36 CMIA0 (8bit Timer 0)
        .data.l INT_CMIB0       //37 CMIB0 (8bit Timer 0)
        .data.l INT_CMIA1B1     //38 CMIA1/CMIB1 (8bit Timer 1)
        .data.l INT_TOVI01      //39 TOVI0/TOVI1 (8bit Timer 0/1)
        .data.l INT_CMIA2       //40 CMIA2 (8bit Timer 2)
        .data.l INT_CMIB2       //41 CMIB2 (8bit Timer 2)
        .data.l INT_CMIA3B3     //42 CMIA3/CMIB3 (8bit Timer 3)
        .data.l INT_TOVI23      //43 TOVI2/TOVI3 (8bit Timer 2/3)
        .data.l INT_DEND0A      //44 DEND0A (DMAC)
        .data.l INT_DEND0B      //45 DEND0B (DMAC)
        .data.l INT_DEND1A      //46 DEND1A (DMAC)
        .data.l INT_DEND1B      //47 DEND1B (DMAC)
        .data.l INT_Reserved    //48 Reserved
        .data.l INT_Reserved    //49 Reserved
        .data.l INT_Reserved    //50 Reserved
        .data.l INT_Reserved    //51 Reserved
        .data.l INT_ERI0        //52 ERI0 (SCI0)
        .data.l INT_RXI0        //53 RXI0 (SCI0)
        .data.l INT_TXI0        //54 TXI0 (SCI0)
        .data.l INT_TEI0        //55 TEI0 (SCI0)
        .data.l INT_ERI1        //56 ERI1 (SCI1)
        .data.l INT_RXI1        //57 RXI1 (SCI1)
        .data.l INT_TXI1        //58 TXI1 (SCI1)
        .data.l INT_TEI1        //59 TEI1 (SCI1)
        .data.l INT_ERI2        //60 ERI2 (SCI2)
        .data.l INT_RXI2        //61 RXI2 (SCI2)
        .data.l INT_TXI2        //62 TXI2 (SCI2)
        .data.l INT_TEI2        //63 TEI2 (SCI2)

INT_NMI:
        rte
INT_TRAP0:
        os_entry IntID_TRAP0
INT_TRAP1:
        os_entry IntID_TRAP1
INT_TRAP2:
        os_entry IntID_TRAP2
INT_TRAP3:
        os_entry IntID_TRAP3
INT_IRQ0:
        os_entry IntID_IRQ0
INT_IRQ1:
        os_entry IntID_IRQ1
INT_IRQ2:
        os_entry IntID_IRQ2
INT_IRQ3:
        os_entry IntID_IRQ3
INT_IRQ4:
        os_entry IntID_IRQ4
INT_IRQ5:
        os_entry IntID_IRQ5
INT_WOVI:
        os_entry IntID_WOVI
INT_CMI:
        os_entry IntID_CMI
INT_ADI:
        os_entry IntID_ADI
INT_IMIA0:
        os_entry IntID_IMIA0
INT_IMIB0:
        os_entry IntID_IMIB0
INT_OVI0:
        os_entry IntID_OVI0
INT_IMIA1:
        os_entry IntID_IMIA1
INT_IMIB1:
        os_entry IntID_IMIB1
INT_OVI1:
        os_entry IntID_OVI1
INT_IMIA2:
        os_entry IntID_IMIA2
INT_IMIB2:
        os_entry IntID_IMIB2
INT_OVI2:
        os_entry IntID_OVI2
INT_CMIA0:
        os_entry IntID_CMIA0
INT_CMIB0:
        os_entry IntID_CMIB0
INT_CMIA1B1:
        os_entry IntID_CMIA1B1
INT_TOVI01:
        os_entry IntID_TOVI01
INT_CMIA2:
        os_entry IntID_CMIA2
INT_CMIB2:
        os_entry IntID_CMIB2
INT_CMIA3B3:
        os_entry IntID_CMIA3B3
INT_TOVI23:
        os_entry IntID_TOVI23
INT_DEND0A:
        os_entry IntID_DEND0A
INT_DEND0B:
        os_entry IntID_DEND0B
INT_DEND1A:
        os_entry IntID_DEND1A
INT_DEND1B:
        os_entry IntID_DEND1B
INT_ERI0:
        os_entry IntID_ERI0
INT_RXI0:
        os_entry IntID_RXI0
INT_TXI0:
        os_entry IntID_TXI0
INT_TEI0:
        os_entry IntID_TEI0
INT_ERI1:
        os_entry IntID_ERI1
INT_RXI1:
        os_entry IntID_RXI1
INT_TXI1:
        os_entry IntID_TXI1
INT_TEI1:
        os_entry IntID_TEI1
INT_ERI2:
        os_entry IntID_ERI2
INT_RXI2:
        os_entry IntID_RXI2
INT_TXI2:
        os_entry IntID_TXI2
INT_TEI2:
        os_entry IntID_TEI2
INT_Reserved:
        os_entry IntID_Reserved

//
// Initialize hardware
//
_start:

// P1 is address bus(A0-7)
        mov.b   #0xff, r0l
        mov.b   r0l, @P1DDR
        
// P2 is address bus(A8-10)
        mov.b   #0x07, r0l
        mov.b   r0l, @P2DDR
        
// P80, CS1-2 enable
        mov.b   #0x0c, r0l
        mov.b   r0l, @P8DDR

// RTCOR 20MHz - 32ms refresh / 2048 raw (15.6us)
        mov.b   #39, r0l
        mov.b   r0l, @RTCOR

// RTMCSR CLK / 8
        mov.b   #0x10, r0l
        mov.b   r0l, @RTMCSR

// DRCRB = MXC1 | RCYCE
        mov.b   #0x90, r0l
        mov.b   r0l, @DRCRB

// DRCRA = DRAS0 
        mov.b   #0x20, r0l
        mov.b   r0l, @DRCRA

// DRAM Initial Wait
        mov.l   #400000, er0
dram_loop:
        subs    #1, er0
        cmp.l   #0, er0
        bne     dram_loop
        
//
// C start up
//

// set SP to end of DRAM
        mov.l   #_stack, sp

// copy DATA
        mov.l   #_idata, er0
        mov.l   #_data, er1
        mov.l   #_edata, er2
data_init_loop:
        cmp     er2, er1
        bhs     bss_init
        mov.l   @er0, er3
        mov.l   er3, @er1
        adds    #4, er0
        adds    #4, er1
        bra     data_init_loop
        
// clear BSS
bss_init:       
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

// init sci get/put for debug
        jsr     @_ini_sci

// call main
        jsr     @_main

// sleep for ever
        orc.b   #0xc0, ccr      //all interrupt disable
        nop
sleep_loop:
        sleep
        bra     sleep_loop

        .end
// end
