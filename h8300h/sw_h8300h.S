//
// sw_h8300h.S : Switch task functions for H8/300H
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
        .globl __kernel_sw
        .type  __kernel_sw, @function
        .globl __kernel_int_entry
        .type  __kernel_int_entry, @function
        .globl __kernel_regpar
        .type  __kernel_regpar, @object

//
// < Task Context Frame : 32 bytes >
//
//      SP +  0 : ER6
//      SP +  4 : ER5
//      SP +  8 : ER4
//      SP + 12 : ER3
//      SP + 16 : ER2
//      SP + 20 : ER1
//      SP + 24 : ER0
//      SP + 28 : CCR&PC
//

//
// T_CTX member offset
//
        .equ    T_CCR,          28      // ccr & pc

//
// T_TCB member offset
//
        .equ    T_SP,           8       // sp
        .equ    T_TEX,          12      // tex

//
// T_VECT member offset
//
        .equ    T_IMASK,        4       // imask

//
// _kernel_sts bits
//
        .equ    BIT_KER_INT,    0       // interrupt executing
        .equ    BIT_KER_SYS,    1       // system task executing
        .equ    BIT_KER_DSP,    3       // dispatch disabled
        
        .text
        .align 2                        //2^2=4

//
// interrupt entry
// arg:
//   er0: exception number (set by gdb stub)
// stack:
//   sp+ 0 : er1        (saved by gdb stub)
//   sp+ 4 : er0        (saved by gdb stub)
//   sp+ 8 : pc & ccr   (saved by exception)
// function:
//   1) interrupt nest counter (_kernel_inest) control
//   2) change sp to interrupt handler stack
//   3) call interrupt handler
//   4) _kernel_sts/T_KER_INT bit control
//
__kernel_int_entry:
        push.l  er2                     //
        push.l  er3                     //
        push.l  er4                     //
        push.l  er5                     //
        push.l  er6                     //

        mov.l   #__kernel_inest, er2    //_kernel_inest++;
        mov.b   @er2, r1l               //
        inc.b   r1l                     //
        mov.b   r1l, @er2               //
        cmp.b   #1, r1l                 //if (_kernel_inest == 1) {
        bne     int_call                //
        mov.l   #__kernel_sts, er2      //  _kernel_sts |= T_KER_INT;
        bset    #BIT_KER_INT, @er2      //
        mov.l   sp, er2                 //  change sp to interrupt stack
        mov.l   @__kernel_int_sp, sp    //
        push.l  er2                     //  save old sp to interrupt stack
int_call:                               //}
        mov.l   er0, er1                //
        shll.l  er1                     //
        mov.l   er1, er2                //
        shll.l  er1                     //
        add.l   er2, er1                //
        add.l   #__kernel_vect, er1     //
        mov.l   @er1, er2               //ER2: FP fp = _kernel_vect[irq].inthdr;
        mov.b   @(T_IMASK, er1), r1l    //CCR = _kernel_vect[irq].imask
        ldc.b   r1l, ccr                //
        jsr     @er2                    //(*fp)(irq);
        mov.b   @__kernel_msk, r0l      //CCR = _kernel_msk;
        ldc.b   r0l, ccr                //
        mov.l   #__kernel_inest, er2    //_kernel_inest--;
        mov.b   @er2, r0l               //
        dec.b   r0l                     //
        mov.b   r0l, @er2               //
        beq     int_end                 //if (_kernel_inest != 0)
        jmp     restore_int             //  goto resotre_int;
int_end:                                //
        pop.l   sp                      //revert sp
        mov.l   #__kernel_sts, er6      //er6: &_kernel_sts
        bclr    #BIT_KER_INT, @er6      //_kernel_sts &= ~T_KER_INT; 

//
// system timer task
// function:
//   1) If _kernel_tic != 0 then call _kernel_tim().
//   2) Change sp to system timer task stack.
//   3) _kernel_sts/T_KER_SYS bit control.
//
sys_tim_task:
        btst    #BIT_KER_SYS, @er6      //if (_kernel_sts & T_KER_SYS)
        bne     restore_int:16          //  goto restore_int;
        mov.b   @__kernel_tic, r0l      //if (!_kernel_tic)
        beq     preempt                 //  goto preempt;
        bset    #BIT_KER_SYS, @er6      //_kernel_sts |= T_KER_SYS;
        mov.b   @(T_CCR, sp), r0h       //R0: UINT psw = CCR of interruptee
        mov.l   sp, er1                 //
        mov.l   @__kernel_tim_sp, sp    //change sp to system task stack
        push.l  er1                     //save old sp to sytem task stack
        jsr     __kernel_tim            //_kernel_tim(psw);
        pop.l   sp                      //revert sp
        bclr    #BIT_KER_SYS, @er6      //_kernel_sts &= ~T_KER_SYS;

//
// preempt dispatcher
// function:
//   Delayed dispatch at end of interrupt.
//   If _kernel_dly && !(_kernel_sts & T_KER_DSP)
//   then dispatch highest priority task.
//
preempt:                                //
        mov.l   #__kernel_cur, er3      //er3: &_kernel_cur
        mov.b   @(T_CCR, sp), r1h       //if (interruptee_CCR_I == 1 ||
        btst    #7, r1h                 //
        bne     preempt_nodisp          //
        mov.l   #__kernel_dly, er5      //    !_kernel_dly ||
        mov.b   @er5, r0l               //
        beq     preempt_nodisp          //
        btst    #BIT_KER_DSP, @er6      //    _kernel_sts & T_KER_DSP) {
        beq     preempt_0               //
preempt_nodisp:                         //
        mov.l   @er3, er0               //    ER0: T_TCB *new = _kernel_cur;
        beq     restore_int             //    if (!new) goto resotre_int;
        bra     chk_tex                 //    goto chk_tex(new, CCR);
preempt_0:                              //}
        mov.b   #0, r0l                 //_kernel_dly = 0; 
        mov.b   r0l, @er5               //
        mov.l   #__kernel_rdq, er2      //er2: T_RDQ *p = _kernel_rdq
        sub.l   er1, er1                //er1: 8
        add.b   #8, r1l                 //
preempt_1:                              //while (p->n == rdq) {
        mov.l   @er2, er0               //  er0: T_TCB *new = p->n;
        cmp.l   er0, er2                //
        bne     preempt_2               //
        add.l   er1, er2                //  p++;
        bra     preempt_1               //}
preempt_2:                              //
        mov.l   @er3, er2               //if (_kernel_cur != NULL) {
        beq     preempt_3               //
        mov.l   sp, @(T_SP, er2)        //  _kernel_cur->sp = sp; 
preempt_3:                              //}
        mov.l   er0, er0                //if (new == NULL)
        beq     idle_task               //  goto idle_task;
        mov.l   er0, @er3               //_kernel_cur = new;
        mov.l   @(T_SP, er0), sp        //sp = new->sp;
        mov.b   #0, r1h                 //psw = 0;
//
// check task exception hanlder
// er0: T_TCB *new;
//  r1: UINT  psw; /* upper 8bits (r1h) : CCR of handler execution */
//
chk_tex:
        mov.l   @(T_TEX, er0), er2      //if (new->tex)
        beq     restore_int             //
        jsr     __kernel_tex            //  _kernel_tex(new, psw);
//
// restore context
//
restore_int:
        pop.l   er6                     //
        pop.l   er5                     //
        pop.l   er4                     //
        pop.l   er3                     //
        pop.l   er2                     //
        pop.l   er1                     //
        pop.l   er0                     //
        rte                             //restore pc & ccr

//
// idle task
//
idle_task:
        //sub.l   er0, er0              //_kernel_cur = NULL;
        mov.l   er0, @__kernel_cur      //
        mov.l   #idle_stack, sp         //set sp to dummy stack
        andc.b  #0x3f, ccr              //enable interrupt
idle_task_1:
        sleep                           //change cpu to sleep mode
        bra     idle_task_1             //sleep again after interrupt
        
//
// void _kernel_sw(T_TCB *new:ER0);
// function:
//   1) clear _kernel_dly
//   2) If new == _kernel_cur && new != NULL then return
//   3) If _kernel_cur != NULL then save current task conext to task stack
//   4) If new == NULL then go to idle task.
//   5) Restore new task context.
//
__kernel_sw:
        mov.b   #0, r1l                 //_kernel_dly = 0;
        mov.b   r1l, @__kernel_dly      //
        mov.l   #__kernel_cur, er3      //er3: &_kernel_cur
        mov.l   @er3, er1               //er1: _kernel_cur
        cmp.l   er0, er1                //if (new == _kernel_cur) {
        bne     kernel_sw_save          //
        mov.l   er0, er0                //  if (new == NULL)
        beq     idle_task               //    goto idle_task;
        rts                             //  return;
kernel_sw_save:                         //}
        mov.l   er1, er1                //if (_kernel_cur != NULL) {
        beq     kernel_sw_disp          //
        mov.b   #0xc0, r2l              //  save CCR (all int disable)
        mov.b   r2l, @sp                //
        mov.l   #__kernel_regpar, er1   //  if (_kernel_regpar.flg) {
        mov.b   @er1, r2l               //
        beq     _kernel_sw_skip         //
        mov.b   #0, r2l                 //    _kernel_regpar.flg = 0;
        mov.b   r2l, @er1               //
        sub.l   #4, sp                  //    skip ER0
        mov.l   @(4, er1), er2          //    save ER1 = _kernel_regpar.er1
        push.l  er2                     //
        mov.l   @(8, er1), er2          //    save ER2 = _kernel_regpar.er2
        push.l  er2                     //
        mov.l   @(12, er1), er2         //    save ER3 = _kernel_regpar.er3
        push.l  er2                     //
        bra     _kernel_sw_er4          //  }
_kernel_sw_skip:                        //  else {
        sub.l   #16, sp                 //    skip ER0-ER3
_kernel_sw_er4:                         //  }
        push.l  er4                     //  save ER4-ER6
        push.l  er5                     //
        push.l  er6                     //
_kernel_sw_sp:                          //  }
        mov.l   @er3, er1               //  _kernel_cur->sp = SP;
        mov.l   sp, @(T_SP, er1)        //
kernel_sw_disp:                         //}
        mov.l   er0, er0                //if (new == NULL)
        beq     idle_task:16            //  goto idle_task;
        mov.l   er0, @er3               //_kernel_cur = new;
        mov.l   @(T_SP, er0), sp        //SP = new->sp;
        mov.b   #0, r1h                 //psw = 0;
        bra     chk_tex:16              //goto chk_tex(new, psw);

//
// work area
//
        .section .bss

        .align  2                       //2^2=4bytes align
__kernel_regpar:
        .space  16                      //TCB register set values (T_REGPAR)
                                        //+ 0:enable flag
                                        //+ 4:set er1 value
                                        //+ 8:set er2 value
                                        //+12:set er3 value
        .space  32                      //dummy stack for idle task
idle_stack:

        .end
//end
