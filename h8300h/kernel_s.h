#ifndef _KERNEL_S_H_
#define _KERNEL_S_H_
/*
 * kernel_s.h : Sub definition of kernel.h for H8/300H
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

#ifdef H8_3069
# include "h8_3069.h"
#endif

/* memory block align */
#define TMSK_ALIGN 1

/* inner ram 1st word : interrupt entry pointer */
#define _KERNEL_INT_ENTRY (*((volatile FP *)H8_INRAM_AD))

/* inner ram 2nd word : extended service call hook */
#define _KERNEL_SVCHOOK (*((_KERNEL_SVC *)(H8_INRAM_AD+4)))

/* number of IRQ */
#define _KERNEL_IRQ_CNT 64

/* interrupt handler IMASK option */
#define _KERNEL_INT_IMASK

typedef struct t_ctx
{
    UW er6;
    UW er5;
    UW er4;
    union {
        UW W;
        struct {
            UH h;
            UH l;
        } H;
    } er3;
    UW er2;
    UW er1;
    UW er0;
    UW ccr_pc;
    UW ret_adr;                 /* jump addr when return from task */
} T_CTX;

#define _KERNEL_INI_CTX() \
{ \
      char *p; \
      T_CTX *sp; \
 \
      p = ((char *)(tcb->stk)) + tcb->ctsk->stksz - sizeof(T_CTX); \
      sp = (T_CTX *)p; \
      sp->er6    = 0;  \
      sp->er5    = 0; \
      sp->er4    = 0; \
      sp->er3.W  = 0; \
      sp->er2    = 0; \
      sp->er1    = 0; \
      sp->er0    = (UW)arg; \
      sp->ccr_pc = (UW)tcb->ctsk->task; \
      sp->ret_adr = (UW)ext_tsk; \
      tcb->sp = sp; \
}

#define _KERNEL_SET_TCB_RET(tcb,ret) {(tcb)->sp->er0 = (UW)(ret);}
#define _KERNEL_SET_TCB_DAT(tcb,dat) {(tcb)->sp->er1 = (UW)(dat);}
#define _KERNEL_SET_TCB_SIZ(tcb,siz) {(tcb)->sp->er2 = (UW)(siz);}
#define _KERNEL_SET_TCB_MOD(tcb,mod) {(tcb)->sp->er2 = (UW)(mod);}
#define _KERNEL_SET_TCB_PTN(tcb,ptn) {(tcb)->sp->er3.H.h = (UH)(ptn);}
#define _KERNEL_SET_TCB_RDV(tcb,rdv) {(tcb)->sp->er3.H.l = (UH)(rdv);}

#define _KERNEL_GET_TCB_RET(tcb) ((ER)    (tcb)->sp->er0)
#define _KERNEL_GET_TCB_DAT(tcb) ((VP_INT)(tcb)->sp->er1)
#define _KERNEL_GET_TCB_SIZ(tcb) ((SIZE)  (tcb)->sp->er2)
#define _KERNEL_GET_TCB_MOD(tcb) ((MODE)  (tcb)->sp->er2)
#define _KERNEL_GET_TCB_PTN(tcb) (        (tcb)->sp->er3.H.h)
#define _KERNEL_GET_TCB_RDV(tcb) ((RDVNO) (tcb)->sp->er3.H.l)

typedef struct t_regpar {
    B flg;
    B dummy1;
    B dummy2;
    B dummy3;
    UW er1;
    UW er2;
    union {
        UW W;
        struct {
            UH h;
            UH l;
        } H;
    } er3;
} T_REGPAR;

extern T_REGPAR _kernel_regpar;

#define _KERNEL_SET_REGPAR_FLG()    {_kernel_regpar.flg = 1;}
#define _KERNEL_SET_REGPAR_DAT(dat) {_kernel_regpar.er1 = (UW)(dat);}
#define _KERNEL_SET_REGPAR_SIZ(siz) {_kernel_regpar.er2 = (UW)(siz);}
#define _KERNEL_SET_REGPAR_MOD(mod) {_kernel_regpar.er2 = (UW)(mod);}
#define _KERNEL_SET_REGPAR_PTN(ptn) {_kernel_regpar.er3.H.h = (UH)(ptn);}
#define _KERNEL_SET_REGPAR_RDV(rdv) {_kernel_regpar.er3.H.l = (UH)(rdv);}

extern void _kernel_int_entry();

#define _KERNEL_INI_SUB() \
{                                                        \
    __asm__ volatile ("ldc.w @__kernel_msk, ccr");       \
    SYSCR &= ~0x08; /* SYSCR UE=0 (2 level interrupt) */ \
    _KERNEL_INT_ENTRY = _kernel_int_entry;                           \
}

extern void _kernel_sw(T_TCB *tcb);

#ifdef _KERNEL_DEBUG
# define _KERNEL_SW(tcb) \
{ \
    if (tcb && !tcb->n) \
        _KERNEL_PANIC(); \
    _kernel_sw(tcb); \
}
#else
# define _KERNEL_SW(tcb) _kernel_sw(tcb)
#endif

/*
 * interrupt mask value of kernel level
 */
extern UINT _kernel_msk;

/*
 * UINT _KERNEL_DIS_PSW();
 *   function: disable interrupt (set kernel level mask to psw)
 *   1) psw = _kernel_msk
 *   2) return previous psw value
 */
static __inline__ UINT __attribute__ ((always_inline))
_KERNEL_DIS_PSW(void)
{
    UINT psw;
    __asm__ volatile ("stc.w ccr, %0" : "=m" (psw));
    __asm__ volatile ("ldc.w @__kernel_msk, ccr");
    return psw;
}

/*
 * void _KERNEL_SET_PSW(UINT psw);
 *   function: set psw
 */
static __inline__ void __attribute__ ((always_inline))
_KERNEL_SET_PSW(UINT psw)
{
    __asm__ volatile ("ldc.w %0, ccr" :: "m" (psw));
}

/*
 * void _KERNEL_DIS();
 *   function: disable interrupt (set kernel level mask to psw)
 *   1) psw = _kernel_msk
 */
#define _KERNEL_DIS() {__asm__ volatile ("ldc.w @__kernel_msk, ccr");}

/*
 * void _KERNEL_DIS_ALL();
 *   function: disable all interrupt
 */
#define _KERNEL_DIS_ALL() {__asm__ volatile ("orc.b #0xc0, ccr");}

/*
 * void _KERNEL_ENA();
 *   function: enable all interrupt
 */
#define _KERNEL_ENA() {__asm__ volatile ("andc.b #0x3f, ccr");}

/*
 * void _KERNEL_INTGAP();
 *   function: gap for intterrupt
 */
#define _KERNEL_INTGAP() {__asm__ volatile ("nop");}

/*
 * interrupt disable level to PSW value
 *   imask : 0,1,2
 */
static __inline__ UINT __attribute__ ((always_inline))
_KERNEL_LVL2PSW(UINT imask)
{
    if (imask == 0) {
        return 0;
    } else if (imask == 1) {
        return 0x8000;
    } else {
        return 0xc000;
    }
}

/*
 * PSW value to interrupt disable level
 */
static __inline__ UINT __attribute__ ((always_inline))
_KERNEL_PSW2LVL(UINT psw)
{
    if (psw & (UINT)0x8000) {
        if (psw & (UINT)0x4000) {
            return 2;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

/*
 * get PSW value
 */
static __inline__ UINT __attribute__ ((always_inline))
_KERNEL_GET_PSW()
{
    UINT psw;
    __asm__ volatile ("stc.w ccr, %0" : "=m" (psw));
    return psw;
}

/*
 * char *_KERNEL_STKTOP();
 */
extern char end;
#define _KERNEL_STKTOP() (&end)

/*
 * char *_KERNEL_STKBOT();
 */
extern char stack;
#define _KERNEL_STKBOT() (&stack)

/*
 * char *_KERNEL_ETEXT();
 */
extern char etext;
#define _KERNEL_ETEXT() (&etext)

static __inline__ VP __attribute__ ((always_inline))
_kernel_pc()
{
    long *frm;
    __asm__ volatile ("mov.l er6, %0" : "=r" (frm));
    return (VP)*(frm + 1);
}

static __inline__ VP __attribute__ ((always_inline))
_KERNEL_GET_SP()
{
    char *sp;
    __asm__ volatile ("mov.l sp, %0" : "=r" (sp));
    return sp;
}

#define _KERNEL_SET_SP(sp) {__asm__ volatile ("mov.l %0, sp" :: "r" (sp));}

#ifdef _KERNEL_DEBUG
# define _KERNEL_PANIC() _kernel_panic(__FILE__, __LINE__)
#else
# define _KERNEL_PANIC() {_KERNEL_DIS();for(;;);}
#endif

#define _KERNEL_TRAP0()    {__asm__ volatile ("trapa #0");}
#define _KERNEL_TRAP1()    {__asm__ volatile ("trapa #1");}
#define _KERNEL_TRAP2()    {__asm__ volatile ("trapa #2");}
#define _KERNEL_TRAP3()    {__asm__ volatile ("trapa #3");}
#define _KERNEL_TRAP_GDB() {__asm__ volatile ("trapa #0");}

#endif /* _KERNEL_S_H_ */
