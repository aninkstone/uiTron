#include        "linkage.h"
#include        "head.inc"
#include        "Interrupts.h"

RDR0 = 0xffffb5
SSR0 = 0xffffb4
RDR1 = 0xffffbd
SSR1 = 0xffffbc
RDR2 = 0xffffc5
SSR2 = 0xffffc4

/* SELECT SIO CH (0/1/2) */
#ifndef CH
# define CH 1
#endif

/*
 * OS interrupt entry reqire :
 *     er0 : exception number
 *     sp+0: pushed ER1
 *     sp+4: pusued ER0
 *     sp+8: pushud CCR & PC
 */

VECT = 0xffbf20

	.macro os_entry intid
        push.l  er0
        push.l  er1
        mov     #\intid, er0
        mov     @VECT, er1
        jmp     @er1
	.endm

;----------------------------------------------------------------------------
        .h8300h
        .file "isr.S"
        .section .text
        .align 2
saveRegisters:
        mov.l   er0, @SYMBOL_NAME(registers)    ; save R0
        mov.l   #SYMBOL_NAME(registers), er0
        mov.l   er1, @(0x04, er0)               ; save R1
        mov.l   er2, @(0x08, er0)               ; save R2
        mov.l   er3, @(0x0c, er0)               ; save R3
        mov.l   er4, @(0x10, er0)               ; save R4
        mov.l   er5, @(0x14, er0)               ; save R5
        mov.l   er6, @(0x18, er0)               ; save R6

        mov.l   #0x0, er1                               ; zero the CCR location
        mov.l   er1, @(0x20, er0)
        mov.w   @er7, r1                                ; get off the stack(CCR)
        mov.b   r1h, @(0x23, er0)               ; save CCR

        mov.l   @er7, er1                               ; get off the stack(CCR & PC)
        and.l   #0xFFFFFF, er1                  ; 24-bit addr
        mov.l   er1, @(0x24, er0)               ; save PC
        
        adds.l  #4, er7
        mov.l   er7, @(0x1c, er0)               ; save R7

        mov.l   @SYMBOL_NAME(stub_sp), er7              ; load __stub_stack
        mov.l   @SYMBOL_NAME(exception), er0
        jsr     SYMBOL_NAME(handle_exception)
        mov.l   er7, @SYMBOL_NAME(stub_sp)              ; save __stub_stack

;----------------------------------------------------------------------------
restoreRegisters:
        mov.l   #SYMBOL_NAME(registers), er0
        mov.l   @(0x1c, er0), er7                               ; restore R7
                mov.l   @(0x24, er0), er1                               ; PC
                sub.w   r2, r2
        mov.b   @(0x23, er0), r2h                               ; CCR
        or.w    r2, e1
                push.l  er1
        mov.l   @(0x18, er0), er6                               ; restore R6
        mov.l   @(0x14, er0), er5                               ; restore R5
        mov.l   @(0x10, er0), er4                               ; restore R4
        mov.l   @(0x0c, er0), er3                               ; restore R3
        mov.l   @(0x08, er0), er2                               ; restore R2
        mov.l   @(0x04, er0), er1                               ; restore R1
        mov.b   #0x00, r0l
        mov.b   r0l, @SYMBOL_NAME(in_nmi)               ; in_nmi = 0        
        mov.l   @SYMBOL_NAME(registers), er0    ; restore R0
        rte                                                                             ; restore CCR & PC

;----------------------------------------------------------------------------
ENTRY(INT_NMI)                          /* NMI */
        push.l  er0
        mov.l   #IntID_NMI, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        mov.b   @er0, r0l
        beq     NEXT
        pop.l   er0
        rte
NEXT:   mov.b   #0x01, r0l
        mov.b   r0l, @SYMBOL_NAME(in_nmi)
        pop.l   er0
        bra     saveRegisters:16

;----------------------------------------------------------------------------
ENTRY(INT_TRAP0)                        /* BREAK */
        push.l  er0
        mov.l   #IntID_TRAP0, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        bra     saveRegisters:16

;----------------------------------------------------------------------------
ENTRY(INT_TRAP1)                        /* BIOS CALL */
        push.l  er0
        mov.l   #IntID_TRAP1, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        bra     saveRegisters:16

;----------------------------------------------------------------------------
ENTRY(INT_TRAP2)
;       push.l  er0
;       mov.l   #IntID_TRAP2, er0
;       mov.l   er0, @SYMBOL_NAME(exception)
;       pop.l   er0
;       bra     saveRegisters:16
        os_entry IntID_TRAP2

;----------------------------------------------------------------------------
ENTRY(INT_TRAP3)                        /* BRAEK */
        push.l  er0
        mov.l   #IntID_TRAP3, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        bra     saveRegisters:16

;----------------------------------------------------------------------------
ENTRY(INT_IRQ0)
        os_entry IntID_IRQ0

;----------------------------------------------------------------------------
ENTRY(INT_IRQ1)
        os_entry IntID_IRQ1

;----------------------------------------------------------------------------
ENTRY(INT_IRQ2)
        os_entry IntID_IRQ2

;----------------------------------------------------------------------------
ENTRY(INT_IRQ3)
        os_entry IntID_IRQ3

;----------------------------------------------------------------------------
ENTRY(INT_IRQ4)
        os_entry IntID_IRQ4

;----------------------------------------------------------------------------
ENTRY(INT_IRQ5)
        os_entry IntID_IRQ5

;----------------------------------------------------------------------------
ENTRY(INT_WOVI)
        os_entry IntID_WOVI

;----------------------------------------------------------------------------
ENTRY(INT_CMI)
        os_entry IntID_CMI

;----------------------------------------------------------------------------
ENTRY(INT_ADI)
        os_entry IntID_ADI

;----------------------------------------------------------------------------
ENTRY(INT_IMIA0)                /* 16 BIT TIMER 0 */
        os_entry IntID_IMIA0

;----------------------------------------------------------------------------
ENTRY(INT_IMIB0)
        os_entry IntID_IMIB0

;----------------------------------------------------------------------------
ENTRY(INT_OVI0)
        os_entry IntID_OVI0

;----------------------------------------------------------------------------
ENTRY(INT_IMIA1)
        os_entry IntID_IMIA1

;----------------------------------------------------------------------------
ENTRY(INT_IMIB1)
        os_entry IntID_IMIB1

;----------------------------------------------------------------------------
ENTRY(INT_OVI1)
        os_entry IntID_OVI1

;----------------------------------------------------------------------------
ENTRY(INT_IMIA2)
        os_entry IntID_IMIA2

;----------------------------------------------------------------------------
ENTRY(INT_IMIB2)
        os_entry IntID_IMIB2

;----------------------------------------------------------------------------
ENTRY(INT_OVI2)
        os_entry IntID_OVI2
        
;----------------------------------------------------------------------------
ENTRY(INT_CMIA0)
        os_entry IntID_CMIA0

;----------------------------------------------------------------------------
ENTRY(INT_CMIB0)
        os_entry IntID_CMIB0

;----------------------------------------------------------------------------
ENTRY(INT_CMIA1B1)
        os_entry IntID_CMIA1B1

;----------------------------------------------------------------------------
ENTRY(INT_TOVI01)
        os_entry IntID_TOVI01

;----------------------------------------------------------------------------
ENTRY(INT_CMIA2)
        os_entry IntID_CMIA2

;----------------------------------------------------------------------------
ENTRY(INT_CMIB2)
        os_entry IntID_CMIB2

;----------------------------------------------------------------------------
ENTRY(INT_CMIA3B3)
        os_entry IntID_CMIA3B3

;----------------------------------------------------------------------------
ENTRY(INT_TOVI23)
        os_entry IntID_TOVI23

;----------------------------------------------------------------------------
ENTRY(INT_DEND0A)
        os_entry IntID_DEND0A

;----------------------------------------------------------------------------
ENTRY(INT_DEND0B)
        os_entry IntID_DEND0B

;----------------------------------------------------------------------------
ENTRY(INT_DEND1A)
        os_entry IntID_DEND1A

;----------------------------------------------------------------------------
ENTRY(INT_DEND1B)
        os_entry IntID_DEND1B

;----------------------------------------------------------------------------
ENTRY(INT_ERI0)
#if CH == 0
        push.l  er0
        mov.l   #IntID_ERI0, er0
        bra     handle_break_0
#else
        os_entry IntID_ERI0
#endif

;----------------------------------------------------------------------------
ENTRY(INT_RXI0)                 /* SCI0 Rx */
#if CH == 0
        push.l  er0
        mov.l   #IntID_RXI0, er0
handle_break_0:
        mov.l   er0, @SYMBOL_NAME(exception)
        mov.b   @RDR0, r0l
        mov.b   @SSR0, r0h
        and.b   #0x80, r0h      /* RDRF & Error clear */
        mov.b   r0h, @SSR0
        cmp.b   #0x03, r0l
        beq     sci_break_0
        pop.l   er0
        rte
sci_break_0:
        pop.l   er0
        bra     saveRegisters:16
#else
        os_entry IntID_RXI0
#endif

;----------------------------------------------------------------------------
ENTRY(INT_TXI0)
#if CH == 0
        push.l  er0
        mov.l   #IntID_TXI0, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        rte
#else
        os_entry IntID_TXI0
#endif

;----------------------------------------------------------------------------
ENTRY(INT_TEI0)
#if CH == 0
        push.l  er0
        mov.l   #IntID_TEI0, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        rte
#else
        os_entry IntID_TEI0
#endif

;----------------------------------------------------------------------------
ENTRY(INT_ERI1)
#if CH == 1
        push.l  er0
        mov.l   #IntID_ERI1, er0
        bra     handle_break_1
#else
        os_entry IntID_ERI1
#endif

;----------------------------------------------------------------------------
ENTRY(INT_RXI1)
#if CH == 1
        push.l  er0
        mov.l   #IntID_RXI1, er0
handle_break_1:
        mov.l   er0, @SYMBOL_NAME(exception)
        mov.b   @RDR1, r0l
        mov.b   @SSR1, r0h
        and.b   #0x80, r0h      /* RDRF & Error clear */
        mov.b   r0h, @SSR1
        cmp.b   #0x03, r0l
        beq     sci_break_1
        pop.l   er0
        rte
sci_break_1:
        pop.l   er0
        bra     saveRegisters:16
#else
        os_entry IntID_RXI1
#endif

;----------------------------------------------------------------------------
ENTRY(INT_TXI1)
#if CH == 1
        push.l  er0
        mov.l   #IntID_TXI1, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        rte
#else
        os_entry IntID_TXI1
#endif

;----------------------------------------------------------------------------
ENTRY(INT_TEI1)
#if CH == 1
        push.l  er0
        mov.l   #IntID_TEI1, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        rte
#else
        os_entry IntID_TEI1
#endif

;----------------------------------------------------------------------------
ENTRY(INT_ERI2)
#if CH == 2
        push.l  er0
        mov.l   #IntID_ERI2, er0
        bra     handle_break_2
#else
        os_entry IntID_ERI2
#endif

;----------------------------------------------------------------------------
ENTRY(INT_RXI2)
#if CH == 2
        push.l  er0
        mov.l   #IntID_RXI2, er0
handle_break_2:
        mov.l   er0, @SYMBOL_NAME(exception)
        mov.b   @RDR2, r0l
        mov.b   @SSR2, r0h
        and.b   #0x80, r0h      /* RDRF & Error clear */
        mov.b   r0h, @SSR2
        cmp.b   #0x03, r0l
        beq     sci_break_2
        pop.l   er0
        rte
sci_break_2:
        pop.l   er0
        bra     saveRegisters:16
#else
        os_entry IntID_RXI2
#endif

;----------------------------------------------------------------------------
ENTRY(INT_TXI2)
#if CH == 2
        push.l  er0
        mov.l   #IntID_TXI2, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        rte
#else
        os_entry IntID_TXI2
#endif

;----------------------------------------------------------------------------
ENTRY(INT_TEI2)
#if CH == 2
        push.l  er0
        mov.l   #IntID_TEI2, er0
        mov.l   er0, @SYMBOL_NAME(exception)
        pop.l   er0
        rte
#else
        os_entry IntID_TEI2
#endif

;----------------------------------------------------------------------------
ENTRY(INT_Reserved)
        os_entry IntID_Reserved

; end
