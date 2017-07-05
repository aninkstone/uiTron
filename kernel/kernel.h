#ifndef _KERNEL_H_
#define _KERNEL_H_
/*
 * kernel.h : Define all of URIBO API
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

typedef void TASK;

#include "itron.h"

/*
 * kernel timer interval value only for apprication
 * ("10" means 10msec)
 */
#ifndef MSEC
# define MSEC 10
#endif

/*
 * size macros
 */
#define TSZ_DTQ(dtqcnt)         (sizeof(VP_INT)*((SIZE)(dtqcnt)))
#define TSZ_DTB(dtqcnt)         (sizeof(UB)*((SIZE)(dtqcnt)))

#define TSZ_MPRIHD(maxmpri)     (sizeof(T_MSGQUE)*((SIZE)(maxmpri)))

#define TSZ_MBF(msgcnt,msgsz)   (((msgsz)==1)?   \
                                 (SIZE)(msgcnt): \
                                 ((SIZE)(msgcnt))*((SIZE)(msgsz)+2))

#define TSZ_PTR(siz)            (((siz)+sizeof(VP)-1)&~(sizeof(VP)-1))

#define TSZ_MPL(cnt,siz)        ((sizeof(T_KERBLK)+TSZ_PTR(siz))*(cnt))

#define TSZ_MPFBLK(bsz)         (((bsz)<sizeof(VP))?sizeof(VP):  \
                                 ((SIZE)(bsz)+sizeof(int)-1)&    \
                                 ~(sizeof(int)-1))

#define TSZ_MPF(cnt,bsz)        (TSZ_MPFBLK(bsz)*(SIZE)(cnt))

#define ID_ANY                  ((ID)-1)

/*
 * for kernel memory block
 */
typedef struct t_kerblk {
    struct t_kerblk *next;      /* link pointer */
    SIZE            len;        /* block length (except block header) */
} T_KERBLK;

typedef struct t_kerblk_head {
    char            *str;       /* start block buffer */
    char            *end;       /* end of block buffer  */
    struct t_kerblk *last;      /* last access point */
    T_KERBLK        zero;       /* header (null) block */
} T_KERBLK_HEAD;

/*
 * bi-directional link list
 */
typedef struct t_lnk {
    struct t_lnk *n;            /* next */
    struct t_lnk *p;            /* prev */
} T_LNK;

/*
 * timer control block and timer queue
 */
typedef struct t_tim {
    struct t_tim *n;            /* next */
    struct t_tim *p;            /* prev */
    TMO          tmo;           /* time out value */
} T_TIM;

typedef struct t_timque {
    UB           msk;           /* hash mask */
    B            rst;           /* request restart */
    T_LNK        *que;          /* hashed que : 1,2,4,8, ... 256 */
} T_TIMQUE;

/*
 * task control block
 */
typedef struct t_tcb {
    struct t_tcb *n;            /* next link pointer */
    struct t_tcb *p;            /* previous link pointer */
    struct t_ctx *sp;           /* stack pointer */
    volatile struct t_tex *tex; /* TEX control block pointer */
    T_TIM        tim;           /* timer control block */
    UB           id;            /* task id */
    UB           pri;           /* current priority */
    UB           sts;           /* task status TTS_??? */
    UB           act;           /* act_tsk counter */
    UB           wup;           /* wup_tsk counter */
    UB           sus;           /* sus_tsk counter */
#ifdef _KERNEL_MTX
    UB           bpri;          /* base priority */
    UB           mtx;           /* loc_mtx counter */
#endif
    STAT         wai;           /* wait type : TTW_??? */
    ID           wid;           /* wait object id */
                                /* above 2 : valid if sts = TTS_WAI/TTS_WAS */
    VP           stk;           /* start of stack */
    T_CTSK       *ctsk;         /* cre_tsk packet */
} T_TCB;

#define TCNV_TIM2TCB(tim) \
    ((T_TCB *)(((char *)(tim))-((int)&((T_TCB *)0)->tim)))

/*
 * semaphore control block
 */
typedef struct t_sem {
    UINT         cnt;
    T_CSEM       *csem;
    T_LNK	 que[1];        /* [1] or [TPRI_MAX] */
    /* T_CSEM    copy; */
} T_SEM;

/*
 * eventflag control block
 */
typedef struct t_flg {
    FLGPTN       flgptn;
    T_CFLG       *cflg;
    T_LNK	que[1];         /* [1] or [TPRI_MAX] */
    /* T_CFLG    copy; */
} T_FLG;

/*
 * data queue control block
 */
typedef struct t_dtq {
    UINT         ip;            /* in  index */
    UINT         op;            /* out index */
    UINT         cnt;           /* data count */
    VP_INT       *buf;
    T_CDTQ       *cdtq;
    T_LNK        rque;          /* receive queue */
    T_LNK        sque[1];       /* send queue, [1] or [TPRI_MAX] */
    /* VP_INT    ring[dtqcnt];                                   */
    /* T_CDTQ    copy;                                           */
} T_DTQ;

/*
 * mailbox control block
 */
typedef struct t_mbxque {
    T_MSG *str;
    T_MSG *end;
} T_MSGQUE;

typedef struct t_mbx {
    T_MSGQUE     *mque;
    T_CMBX       *cmbx;
    T_LNK	 que[1];        /* [1] or [TPRI_MAX] */
    /* T_MSGQUE  msgque[]          [1] or [maxmpri]; */
    /* T_CDTQ    copy;                               */
} T_MBX;

/*
 * mutex control block
 */
typedef struct t_mtx {
    T_TCB        *loc;
    T_CMTX       *cmtx;
    T_LNK	 que[1];        /* [1] or [TPRI_MAX] */
} T_MTX;

/*
 * message buffer control block
 */
typedef struct t_mbf {
    UB           *buf;
    UB           *in;
    UB           *out;
    SIZE         sz;
    UINT         smsgcnt;
    T_CMBF       *cmbf;
    T_LNK        sque[1];       /* [1] or [TPRI_MAX] */
    /* T_LNK     rque[];           [1] or [TPRI_MAX] */
    /* T_CMBF    copy;                               */
} T_MBF;

/*
 * rendezvous port control block
 */
typedef struct t_por {
    T_CPOR       *cpor;
    T_LNK        aque;          /* accept queue */
    T_LNK        cque[1];       /* call queue, [1] or [TPRI_MAX] */
    /* T_CPOR    copy;                                           */
} T_POR;

/*
 * variable-sized memory pool control block
 */
typedef struct t_mpl {
    T_KERBLK_HEAD head;
    T_CMPL       *cmpl;
    T_LNK        que[1];        /* [1] or [TPRI_MAX] */
} T_MPL;

/*
 * fixed-sized memory pool control block
 */
typedef struct t_mpf {
    T_MSGQUE     free;          /* free buffer link */
    UINT         cnt;           /* free block count */
    char         *str;          /* start block buffer */
    char         *end;          /* end of block buffer  */
    T_CMPF       *cmpf;
    T_LNK        que[1];        /* [1] or [TPRI_MAX] */
} T_MPF;

/*
 * cyclic handler control block
 */
typedef struct t_cyc {
    T_TIM        tim;
    UB           sta;
    T_CCYC       *ccyc;
} T_CYC;

/*
 * alarm handler control block
 */
typedef struct t_alm {
    T_TIM        tim;
    T_CALM       *calm;
} T_ALM;

/*
 * extended service call control block
 */
typedef struct t_svc {
    FP           svcrtn;
    INT          parn;
} T_SVC;

/*
 * task exception handler control block
 */
typedef struct t_tex {
    TEXPTN       ptn;
    STAT         stat;
    T_DTEX       *dtex;
} T_TEX;

typedef void (*FP_TEX)(TEXPTN, VP_INT); /* tex handler */

/*
 * ref_sys packet
 */
typedef struct t_rsys {
    INT          sysstat;
} T_RSYS;

#define TSS_TSK  0
#define TSS_DDSP 1
#define TSS_LOC  3
#define TSS_INDP 4

/*
 * ref_cfg packet
 */
typedef struct t_rcfg {
    ID           tskid_max;
    ID           semid_max;
    ID           flgid_max;
    ID           mbxid_max;
    ID           mbfid_max;
    ID           porid_max;
    ID           mplid_max;
    ID           mpfid_max;
    ID           cycno_max;
    ID           almno_max;
    PRI          tpri_max;
    int          tmrqsz;
    int          cycqsz;
    int          almqsz;
    int          istksz;
    int          tstksz;
    SIZE         sysmsz;
    SIZE         mplmsz;
    SIZE         stkmsz;
    ID           dtqid_max;
    ID           mtxid_max;
    ID           isrid_max;
    ID           svcfn_max;
    int          isrqsz;
} T_RCFG;

/*
 * kernel variables
 */
extern volatile T_TCB *_kernel_cur; /* current task TCB */

extern volatile UB _kernel_sts; /* kernel status */
#define T_KER_INT     0x01      /* kernel interrupt handler executing */
#define T_KER_SYS     0x02      /* system task executing */
#define T_KER_LOC     0x04      /* cpu locked */
#define T_KER_DSP     0x08      /* dispatch disabled */

extern volatile UB _kernel_dly;         /* 1:delayed dispatch request */
extern volatile UB _kernel_tic;         /* kernel tic cnt */
extern volatile TMO _kernel_sysclk;     /* kernel system clock */
extern volatile SYSTIM _kernel_systim;  /* system time */
extern T_TIMQUE _kernel_tmrq;           /* task timer queue */
extern T_TIMQUE _kernel_cycq;           /* cyclic handler queue */
extern T_TIMQUE _kernel_almq;           /* alarm handler queue */
extern SIZE _kernel_istksz;     /* interrupt handler stack size */
extern char *_kernel_int_sp;    /* interrupt handler initial sp */
extern SIZE _kernel_tstksz;     /* system timer task stack size */
extern char *_kernel_tim_sp;    /* system timer task initial sp */
extern ER SYSER;                /* system call error code log */
extern UB _kernel_inest;        /* interrupt nest counter */
extern B _kernel_rdvseq;        /* rendezvous sequential number */
extern T_LNK _kernel_dmy_que;   /* dummy waiting queue */
extern volatile OVRTIM *_kernel_ovrcnt; /* overrun counter */
extern volatile B _kernel_ovrreq;       /* request overrun handler */
extern volatile FP _kernel_ovrhdr;      /* overrun handler address */
extern B (*_kernel_chg_mtx_fp)(T_TCB *tcb, PRI pri); /* _kernel_chg_mtx() hook */
extern B (*_kernel_chk_wmbf_fp)(T_MBF *mbf);  /* _kernel_chk_wmbf() hook */
extern B (*_kernel_chk_wmpl_fp)(T_MPL *mpl);  /* _kernel_chk_wmpl() hook */
extern B (*_kernel_unl_amtx_fp)(T_TCB *atcb); /* _kernel_unl_amtx() hook */
extern T_LNK _kernel_isrq[];            /* ISR hashed queue */
extern UH _kernel_isrq_msk;             /* ISR queue hash mask */
extern UINT _kernel_level;      /* critical section disable interrupt level */
extern UINT _kernel_level_max;  /* maximum interrupt level */
extern struct t_vect _kernel_vect[];    /* pesudo interrupt vector */

#ifdef _KERNEL_MTX
extern B *_kernel_inherit;      /* task inherit map for mutex */
#endif /* _KERNEL_MTX */

extern PRI _kernel_tpri_max;
extern UINT _kernel_tskid_max;
extern UINT _kernel_semid_max;
extern UINT _kernel_flgid_max;
extern UINT _kernel_dtqid_max;
extern UINT _kernel_mbxid_max;
extern UINT _kernel_mtxid_max;
extern UINT _kernel_mbfid_max;
extern UINT _kernel_porid_max;
extern UINT _kernel_isrid_max;
extern UINT _kernel_mplid_max;
extern UINT _kernel_mpfid_max;
extern UINT _kernel_cycno_max;
extern UINT _kernel_almno_max;
extern UINT _kernel_svcfn_max;

extern T_LNK  _kernel_rdq[];
extern T_TCB *_kernel_tcb[];
extern T_SEM *_kernel_sem[];
extern T_FLG *_kernel_flg[];
extern T_DTQ *_kernel_dtq[];
extern T_MBX *_kernel_mbx[];
extern T_MTX *_kernel_mtx[];
extern T_MBF *_kernel_mbf[];
extern T_POR *_kernel_por[];
extern T_MPL *_kernel_mpl[];
extern T_MPF *_kernel_mpf[];
extern T_CYC *_kernel_cyc[];
extern T_ALM *_kernel_alm[];
extern T_SVC *_kernel_svc[];

extern SIZE _kernel_sysmsz;           /* system memory size */
extern SIZE _kernel_mplmsz;           /* memory pool memory size */
extern SIZE _kernel_stkmsz;           /* stack memory size */
extern T_KERBLK_HEAD *_kernel_sysmem; /* system memory pool header */
extern T_KERBLK_HEAD *_kernel_mplmem; /* mpl memory pool header */
extern T_KERBLK_HEAD _kernel_stkmem;  /* stack memory pool header */

extern T_LNK _kernel_irq[];     /* IRQ table (for ISR only) */

/*
 * kernel functions
 */
char *_kernel_er(ER er);
void _kernel_nul_que(T_LNK *que, PRI pri);
void _kernel_enq(T_LNK *que, T_LNK *add);
void _kernel_deq(T_LNK *del);
T_LNK *_kernel_first(T_LNK *que, PRI pri);
void _kernel_run(T_TCB *new);
void _kernel_higher(T_TCB *new);
void _kernel_highest(void);
ER _kernel_wai(T_TCB *tcb, TMO tmout, T_LNK *que, BOOL pri);
void _kernel_deq_rdy_dsp(T_TCB *tcb);
B _kernel_deq_rdy(T_TCB *tcb);
void _kernel_dlt(T_LNK *que, PRI pri);
void _kernel_act(T_TCB *tcb, VP_INT arg);
void _kernel_tex(T_TCB *tcb, UINT psw);
void _kernel_tim(UINT psw);
void _kernel_ini_mem(T_KERBLK_HEAD *head, char *adr, SIZE siz);
char *_kernel_alloc(T_KERBLK_HEAD *head, SIZE siz);
ER _kernel_free(T_KERBLK_HEAD *head, char *adr);
char *_kernel_sys_alloc(SIZE siz);
char *_kernel_mpl_alloc(SIZE siz);
char *_kernel_stk_alloc(SIZE siz);
void _kernel_sys_free(VP mem);
void _kernel_mpl_free(VP mem);
void _kernel_stk_free(VP mem);
B _kernel_dmy_hook();
void _kernel_nul_msg_que(T_MSGQUE *que, PRI pri);
void _kernel_enq_msg(T_MSGQUE *que, T_MSG *msg, PRI pri);
T_MSG *_kernel_deq_msg(T_MSGQUE *que, PRI pri);
T_MSG *_kernel_first_msg(T_MSGQUE *que, PRI pri);

B _kernel_chg_mtx(T_TCB *tcb, PRI pri);
B _kernel_chg_pri(T_TCB *tcb, PRI pri);

UINT _kernel_put_mbf(T_MBF *mbf, UB *msg, UINT msgsz);
B _kernel_chk_wmbf(T_MBF *mbf);

B _kernel_chk_wmpl(T_MPL *mpl);

B _kernel_unl_amtx(T_TCB *atcb);

/*
 * system calls
 */
ER sysini(void);
ER syssta(void);
ER intsta(void);
ER intext(void);
ER cre_tsk(ID tskid, const T_CTSK *pk_ctsk);
#define acre_tsk(pk_ctsk)     cre_tsk(ID_ANY,(pk_ctsk))
ER del_tsk(ID tskid);
ER act_tsk(ID tskid);
#define iact_tsk           act_tsk
ER_UINT can_act(ID tskid);
ER sta_tsk(ID tskid, VP_INT stacd);
void ext_tsk();
void exd_tsk();
ER ter_tsk(ID tskid);
ER chg_pri(ID tskid, PRI tskpri);
ER get_pri(ID tskid, PRI *p_tskpri);
ER ref_tsk(ID tskid, T_RTSK *pk_rtsk);
ER ref_tst(ID tskid, T_RTST *pk_rtst);

ER sus_tsk(ID tskid);
ER rsm_tsk(ID tskid);
ER frsm_tsk(ID tskid);
#define slp_tsk()          tslp_tsk(TMO_FEVR)
ER tslp_tsk(TMO tmout);
ER wup_tsk(ID tskid);
#define iwup_tsk           wup_tsk
ER_UINT can_wup(ID tskid);
ER rel_wai(ID tskid);
#define irel_wai           rel_wai
ER dly_tsk(RELTIM dlytim);

ER def_tex(ID tskid, const T_DTEX *pk_dtex);
ER ras_tex(ID tskid, TEXPTN rasptn);
#define iras_tex           ras_tex
ER dis_tex();
ER ena_tex();
BOOL sns_tex();
ER ref_tex(ID tskid, T_RTEX *pk_rtex);

ER cre_sem(ID semid, const T_CSEM *pk_csem);
#define acre_sem(csem)     cre_sem(ID_ANY,(csem))
ER del_sem(ID smdid);
ER sig_sem(ID semid);
#define isig_sem           sig_sem
#define wai_sem(semid)     twai_sem((semid),TMO_FEVR)
#define pol_sem(semid)     twai_sem((semid),TMO_POL)
ER twai_sem(ID semid, TMO tmout);
ER ref_sem(ID semid, T_RSEM *pk_rsem);

ER cre_flg(ID flgid, const T_CFLG *pk_cflg);
#define acre_flg(cflg)     cre_flg(ID_ANY,(cflg))
ER del_flg(ID flgid);
ER set_flg(ID flgid, FLGPTN setptn);
#define iset_flg           set_flg
ER clr_flg(ID flgid, FLGPTN clrptn);
#define wai_flg(flgid,ptn,mod,p_ptn) \
        twai_flg((flgid),(ptn),(mod),(p_ptn),TMO_FEVR)
#define pol_flg(flgid,ptn,mod,p_ptn) \
        twai_flg((flgid),(ptn),(mod),(p_ptn),TMO_POL)
ER twai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN*p_flgptn, TMO tmout);
ER ref_flg(ID flgid, T_RFLG *pk_rflg);

ER cre_dtq(ID dtqid, const T_CDTQ *pk_cdtq);
#define acre_dtq(cdtq)     cre_dtq(ID_ANY,(cdtq))
ER del_dtq(ID dtqid);
#define snd_dtq(dtqid,dat)    tsnd_dtq((dtqid),(dat),TMO_FEVR)
#define psnd_dtq(dtqid,dat)   tsnd_dtq((dtqid),(dat),TMO_POL)
#define ipsnd_dtq(dtqid,dat)  tsnd_dtq((dtqid),(dat),TMO_POL)
ER tsnd_dtq(ID dtqid, VP_INT data, TMO tmout);
ER fsnd_dtq(ID dtqid, VP_INT data);
#define ifsnd_dtq          fsnd_dtq
#define rcv_dtq(dtqid,p_dat)  trcv_dtq((dtqid),(p_dat),TMO_FEVR)
#define prcv_dtq(dtqid,p_dat) trcv_dtq((dtqid),(p_dat),TMO_POL)
ER trcv_dtq(ID dtqid, VP_INT *p_data, TMO tmout);
ER ref_dtq(ID dtqid, T_RDTQ *pk_rdtq);

ER cre_mbx(ID mbxid, const T_CMBX *pk_cmbx);
#define acre_mbx(cmbx)     cre_mbx(ID_ANY,(cmbx))
ER del_mbx(ID mbxid);
ER snd_mbx(ID mbxid, T_MSG *pk_msg);
#define rcv_mbx(id,msg)    trcv_mbx((id),(msg),TMO_FEVR)
#define prcv_mbx(id,msg)   trcv_mbx((id),(msg),TMO_POL)
ER trcv_mbx(ID mbxid, T_MSG **ppk_msg, TMO tmout);
ER ref_mbx(ID mbxid, T_RMBX *pk_rmbx);

ER cre_mtx(ID mtxid, const T_CMTX *pk_cmtx);
#define acre_mtx(cmtx)     cre_dtq(ID_ANY,(cmtx))
ER del_mtx(ID mtxid);
ER unl_mtx(ID mtxid);
#define loc_mtx(mtxid)     tloc_mtx((mtxid),TMO_FEVR)
#define ploc_mtx(mtxid)    tloc_mtx((mtxid),TMO_POL)
ER tloc_mtx(ID mtxid, TMO tmout);
ER ref_mtx(ID mtxid, T_RMTX *pk_rmtx);

ER cre_mbf(ID mbfid, const T_CMBF *pk_cmbf);
#define acre_mbf(cmbf)     cre_mbf(ID_ANY,(cmbf))
ER del_mbf(ID mbfid);
#define snd_mbf(mbfid,msg,msgsz)  tsnd_mbf((mbfid),(msg),(msgsz),TMO_FEVR);
#define psnd_mbf(mbfid,msg,msgsz) tsnd_mbf((mbfid),(msg),(msgsz),TMO_POL);
ER tsnd_mbf(ID mbfid, VP msg, UINT msgsz, TMO tmout);
#define rcv_mbf(mbfid,msg)  trcv_mbf((mbfid),(msg),TMO_FEVR);
#define prcv_mbf(mbfid,msg) trcv_mbf((mbfid),(msg),TMO_POL);
ER_UINT trcv_mbf(ID mbfid, VP msg, TMO tmout);
ER ref_mbf(ID mbfid, T_RMBF *pk_rmbf);

ER cre_por(ID porid, const T_CPOR *pk_cpor);
#define acre_por(cpor)     cre_por(ID_ANY,(cpor))
ER del_por(ID porid);
ER_UINT tcal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmout);
#define cal_por(porid,calptn,msg,cmsgsz) \
    tcal_por((porid),(calptn),(msg),(cmsgsz),TMO_FEVR) 
ER_UINT tacp_por(ID porid, RDVPTN acpptn, RDVNO *p_rdvno, VP msg, TMO tmout);
#define acp_por(porid,acpptn,p_rdvno,msg) \
    tacp_por((porid),(acpptn),(p_rdvno),(msg),TMO_FEVR)
#define pacp_por(porid,acpptn,p_rdvno,msg) \
    tacp_por((porid),(acpptn),(p_rdvno),(msg),TMO_POL)
ER fwd_por(ID porid, RDVPTN calptn, RDVNO rdvno, VP msg, UINT cmsgsz);
ER rpl_rdv(RDVNO rdvno, VP msg, UINT rmsgsz);
ER ref_por(ID porid, T_RPOR *pk_rpor);
ER ref_rdv(RDVNO rdvno, T_RRDV *pk_rrdv);

ER def_inh(INHNO inhno, const T_DINH *pk_dinh);
#define ent_int() {}
#define ient_int() {}
#define ret_int() {}
#define iret_int() {}
ER chg_ims(UINT imask);
ER get_ims(UINT *p_imask);
#define vdis_psw _KERNEL_DIS_PSW
#define vset_psw _KERNEL_SET_PSW
ER cre_isr(ID isrid, const T_CISR *pk_cisr);
#define acre_isr(cisr)     cre_isr(ID_ANY,(cisr))
ER del_isr(ID isrid);
ER ref_isr(ID isrid, T_RISR *pk_risr);
ER dis_int(INTNO intno);
ER ena_int(INTNO intno);

ER cre_mpl(ID mplid, const T_CMPL *pk_cmpl);
#define acre_mpl(cmpl)     cre_mpl(ID_ANY,(cmpl))
ER del_mpl(ID mplid);
ER tget_mpl(ID mplid, UINT blksz, VP *p_blk, TMO tmout);
#define get_mpl(mplid,blksz,p_blk) \
    tget_mpl((mplid),(blksz),(p_blk),TMO_FEVR));
#define pget_mpl(mplid,blksz,p_blk) \
    tget_mpl((mplid),(blksz),(p_blk),TMO_POL));
ER rel_mpl(ID mplid, VP blk);

ER cre_mpf(ID mpfid, const T_CMPF *pk_cmpf);
#define acre_mpf(cmpf)     cre_mpf(ID_ANY,(cmpf))
ER del_mpf(ID mpfid);
ER tget_mpf(ID mpfid, VP *p_blf, TMO tmout);
#define get_mpf(id,p_blf)  tget_mpf((id),(p_blf),TMO_FEVR)
#define pget_mpf(id,p_blf) tget_mpf((id),(p_blf),TMO_POL)
ER rel_mpf(ID mpfid, VP blf);
ER ref_mpf(ID mpfid, T_RMPF *pk_rmpf);

ER set_tim(SYSTIM *p_systim);
ER get_tim(SYSTIM *p_systim);

ER cre_cyc(ID cycid, const T_CCYC *pk_ccyc);
#define acre_cyc(ccyc)     cre_cyc(ID_ANY,(ccyc))
ER del_cyc(ID cycid);
ER sta_cyc(ID cycid);
ER stp_cyc(ID cycid);
ER ref_cyc(ID cycid, T_RCYC *pk_rcyc);

ER cre_alm(ID almid, const T_CALM *pk_calm);
#define acre_alm(calm)     cre_alm(ID_ANY,(calm))
ER del_alm(ID almid);
ER sta_alm(ID almid, RELTIM almtim);
ER stp_alm(ID almid);
ER ref_alm(ID almid, T_RALM *pk_ralm);

ER def_ovr(T_DOVR *pk_dovr);
ER sta_ovr(ID tskid, OVRTIM ovrtim);
ER stp_ovr(ID tskid);
ER ref_ovr(ID tskid, T_ROVR *pk_rovr);

ER def_svc(FN fncd, const T_DSVC *pk_dsvc);

ER rot_rdq(PRI tskpri);
#define irot_rdq           rot_rdq
ER get_tid(ID *p_tskid);
#define iget_tid           get_tid
#define iloc_cpu           loc_cpu
#define iunl_cpu           unl_cpu
ER dis_dsp();
ER ena_dsp();
#define sns_ctx()          (_kernel_sts & (T_KER_INT | T_KER_SYS))
#define sns_loc()          (_kernel_sts & T_KER_LOC)
#define sns_dsp()          (_kernel_sts & T_KER_DSP)
#define sns_dpn()          (_kernel_sts & (T_KER_INT | T_KER_SYS | T_KER_LOC | T_KER_DSP))
ER ref_sys(ID id, T_RSYS *pk_rsys);

ER ref_cfg(T_RCFG *pk_rcfg);
ER ref_ver(T_RVER *pk_rver);

#define _KERNEL_RET(c)       {r = SYSER = (c); goto ret;}
#define _KERNEL_END(c)       {r = SYSER = (c); goto end;}
#define _KERNEL_END_NOLOG(c) {r =         (c); goto end;}

#define _KERNEL_CHK_LOC(s) ((s) & T_KER_LOC)

#define _KERNEL_CHK_DSP(s) ((s) & T_KER_DSP)

#define _KERNEL_CHK_SYS(s) ((s) & T_KER_SYS)

#define _KERNEL_CHK_IS(s) ((s) & (T_KER_INT | T_KER_SYS))

#define _KERNEL_CHK_IL(s) ((s) & (T_KER_INT | T_KER_LOC))

#define _KERNEL_CHK_ISL(s) ((s) & (T_KER_INT | T_KER_SYS | T_KER_LOC))

#define _KERNEL_CHK_ISD(s) ((s) & (T_KER_INT | T_KER_SYS | T_KER_DSP))

#define _KERNEL_CHK_ISLD(s) \
((s) & (T_KER_INT | T_KER_SYS | T_KER_LOC | T_KER_DSP))

static __inline__ int __attribute__ ((always_inline))
_KERNEL_CHK_WAI(UB s, TMO t)
{
    return ((s) & (T_KER_INT | T_KER_SYS | T_KER_LOC) ||
            (((s) & T_KER_DSP) && ((t) != TMO_POL))    )? 1 : 0;
}

#define _KERNEL_CHK_TSK(s) (!((s) & (T_KER_INT | T_KER_SYS)))

#define _KERNEL_DATA __attribute__((section (".data")))

#define _KERNEL_ALIGN(ptr) (((unsigned long)(ptr)+sizeof(VP)-1)&~(sizeof(VP)-1))
#define _KERNEL_ALIGN_BACK(ptr) (((unsigned long)(ptr))&~(sizeof(VP)-1))

/*
 * vcan_wup inlined system call
 */
static __inline__ void __attribute__ ((always_inline))
vcan_wup()
{
    if (_KERNEL_CHK_TSK(_kernel_sts))
        _kernel_cur->wup = 0;
}

/*
 * vget_tid inlined system call
 */
static __inline__ ID __attribute__ ((always_inline))
vget_tid()
{
    return (!_kernel_cur || _KERNEL_CHK_SYS(_kernel_sts))?
        TSK_NONE : _kernel_cur->id;
}

/*
 * isig_tim inlined system call
 */
static __inline__ void __attribute__ ((always_inline))
isig_tim()
{                      
    /* if (_kernel_tic != 255) _kernel_tic++; */
    UB t = _kernel_tic + 1;
    if (t)
        _kernel_tic = t;
}

/*
 * extended service call
 */
typedef ER_UINT (*_KERNEL_SVC)(FN fncd, ...);
#define cal_svc _KERNEL_SVCHOOK

/*
 * timer queue control functions
 */
static __inline__ void __attribute__ ((always_inline))
_kernel_enq_tmq(T_TIMQUE *tmq, T_TIM *tim)
{
    _kernel_enq(&tmq->que[tim->tmo & tmq->msk], (T_LNK *)tim);
    tmq->rst = 1;
}

static __inline__ void __attribute__ ((always_inline))
_kernel_deq_tmq(T_TIMQUE *tmq, T_TIM *tim)
{
    _kernel_deq((T_LNK *)tim);
    tmq->rst = 1;
}

#include "kernel_s.h"

/*
 * loc_cpu inlined system call
 */
static __inline__ ER __attribute__ ((always_inline))
loc_cpu(void)
{
    _KERNEL_DIS();
    _kernel_sts |= T_KER_LOC;
    return E_OK;
}

/*
 * unl_cpu inlined system call
 */
static __inline__ ER __attribute__ ((always_inline))
unl_cpu(void)
{
    _KERNEL_DIS();
    _kernel_sts &= ~T_KER_LOC;
    _KERNEL_ENA();
    return E_OK;
}

typedef void (*FP_INH)(INHNO);

typedef struct t_vect {
    FP_INH inthdr;
#ifdef _KERNEL_INT_IMASK
    UINT   imask;
#endif
} T_VECT;

typedef void (*FP_ISR)(VP_INT, INHNO);

typedef struct t_isr {
    struct t_lnk *n;
    struct t_lnk *p;
    FP_ISR       isr;
    VP_INT       exinf;
    INTNO        intno;
#ifdef _KERNEL_INT_IMASK
    UINT         imask;
#endif
} T_ISR;

extern T_ISR *_kernel_isr[];

/*
 * system call assertion
 */

#ifdef _KERNEL_ASSERT
# include "tprintf.h"
# define _KERNEL_ASSERT_CRE_TSK()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "cre_tsk(tskid=%d, pk_ctsk=0x%x) tskatr=0x%x, "  \
            "exinf=0x%x, task=0x%x, itskpri=%d, stksz=%d, "  \
            "stk=0x%x, name=%s\n",                           \
            _kernel_pc(), _kernel_er(r),                     \
            tskid, pk_ctsk, pk_ctsk->tskatr, pk_ctsk->exinf, \
            pk_ctsk->task, pk_ctsk->itskpri, pk_ctsk->stksz, \
            pk_ctsk->stk, pk_ctsk->name?pk_ctsk->name:"");   \
    }
# define _KERNEL_ASSERT_DEL_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_tsk(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_ACT_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "act_tsk(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_CAN_ACT()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "can_act(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_STA_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sta_tsk(tskid=%d, stacd=0x%x)\n",               \
            _kernel_pc(), _kernel_er(r), tskid, stacd);      \
    }
# define _KERNEL_ASSERT_EXT_TSK()  \
    _kernel_assert("pc=(may be)0x%x "                               \
        "E_CTX tskid=%d ext_tsk()\n",                               \
        _kernel_pc(), _KERNEL_CHK_IS(_kernel_sts)?0:_kernel_cur->id);
# define _KERNEL_ASSERT_EXD_TSK()  \
    _kernel_assert("pc=(may be)0x%x "                               \
        "E_CTX tskid=%d exd_tsk()\n",                               \
        _kernel_pc(), _KERNEL_CHK_IS(_kernel_sts)?0:_kernel_cur->id);
# define _KERNEL_ASSERT_TER_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ter_tsk(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_CHG_PRI()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "chg_pri(tskid=%d, tskpri=%d)\n",                \
            _kernel_pc(), _kernel_er(r), tskid, tskpri);     \
    }
# define _KERNEL_ASSERT_GET_PRI()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "get_pri(tskid=%d, p_tskpri=0x%x)\n",            \
            _kernel_pc(), _kernel_er(r), tskid, p_tskpri);   \
    }
# define _KERNEL_ASSERT_REF_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_tsk(tskid=%d, pk_rtsk=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), tskid, pk_rtsk);    \
    }
# define _KERNEL_ASSERT_REF_TST()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_tst(tskid=%d, pk_rtst=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), tskid, pk_rtst);    \
    }
# define _KERNEL_ASSERT_SUS_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sus_tsk(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_RSM_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "rsm_tsk(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_FRSM_TSK() \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "frsm_tsk(tskid=%d)\n",                          \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_TSLP_TSK() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI) {         \
        _kernel_assert("pc=0x%x er=%s "                      \
            "tslp_tsk(tmout=%d)\n",                          \
            _kernel_pc(), _kernel_er(r), tmout);             \
    }
# define _KERNEL_ASSERT_WUP_TSK()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "wup_tsk(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_CAN_WUP()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "can_wup(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_REL_WAI()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "rel_wai(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_DLY_TSK()  \
    if (r != E_OK && r != E_RLWAI) {                         \
        _kernel_assert("pc=0x%x er=%s "                      \
            "dly_tsk(tmout=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tmout);             \
    }
# define _KERNEL_ASSERT_DEF_TEX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "def_tex(tskid=%d, pk_dtex=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), tskid, pk_dtex);    \
    }
# define _KERNEL_ASSERT_RAS_TEX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ras_tex(tskid=%d, rasptn=0x%x)\n",              \
            _kernel_pc(), _kernel_er(r), tskid, rasptn);     \
    }
# define _KERNEL_ASSERT_DIS_TEX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "dis_tex()\n",                                   \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_ENA_TEX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ena_tex()\n",                                   \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_REF_TEX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_tex(tskid=%d, pk_rtex=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), tskid, pk_rtex);    \
    }
# define _KERNEL_ASSERT_CRE_SEM()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "cre_sem(semid=%d, pk_csem=0x%x) "               \
            "sematr=0x%x, isemcnt=%d, maxsem=%d, name=%s\n", \
            _kernel_pc(), _kernel_er(r),                     \
            semid, pk_csem,                                  \
            pk_csem->sematr, pk_csem->isemcnt,               \
            pk_csem->maxsem, pk_csem->name?pk_csem->name:"");\
    }
# define _KERNEL_ASSERT_DEL_SEM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_sem(semid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_SIG_SEM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sig_sem(semid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_TWAI_SEM() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "twai_sem(semid=%d, tmout=%d)\n",                      \
            _kernel_pc(), _kernel_er(r), id, tmout);               \
    }
# define _KERNEL_ASSERT_REF_SEM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_sem(semid=%d, pk_rsem=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rsem);       \
    }
# define _KERNEL_ASSERT_CRE_FLG()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "cre_flg(flgid=%d, pk_cflg=0x%x) "               \
            "flgatr=0x%x, iflgptn=0x%x, name=%s\n",          \
            _kernel_pc(), _kernel_er(r),                     \
            flgid, pk_cflg,                                  \
            pk_cflg->flgatr, pk_cflg->iflgptn,               \
            pk_cflg->name?pk_cflg->name:"");                 \
    }
# define _KERNEL_ASSERT_DEL_FLG()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_flg(flgid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_SET_FLG()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "set_flg(flgid=%d, setptn=0x%x)\n",              \
            _kernel_pc(), _kernel_er(r), id, setptn);        \
    }
# define _KERNEL_ASSERT_CLR_FLG()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "clr_flg(flgid=%d, clrptn=0x%x)\n",              \
            _kernel_pc(), _kernel_er(r), id, clrptn);        \
    }
# define _KERNEL_ASSERT_TWAI_FLG() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "twai_flg(flgid=%d, tmout=%d)\n",                      \
            _kernel_pc(), _kernel_er(r), id, tmout);               \
    }
# define _KERNEL_ASSERT_REF_FLG()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_flg(flgid=%d, pk_rflg=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rflg);       \
    }
# define _KERNEL_ASSERT_CRE_DTQ()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "cre_dtq(dtqid=%d, pk_cdtq=0x%x) "               \
            "dtqatr=0x%x, dtqcnt=%d dtq=0x%x, name=%s\n",    \
            _kernel_pc(), _kernel_er(r),                     \
            dtqid, pk_cdtq,                                  \
            pk_cdtq->dtqatr, pk_cdtq->dtqcnt, pk_cdtq->dtq,  \
            pk_cdtq->name?pk_cdtq->name:"");                 \
    }
# define _KERNEL_ASSERT_DEL_DTQ()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_dtq(dtqid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_TSND_DTQ() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tsnd_dtq(dtqid=%d, data=0x%x, tmout=%d)\n",           \
            _kernel_pc(), _kernel_er(r), id, data, tmout);         \
    }
# define _KERNEL_ASSERT_FSND_DTQ() \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "fsnd_dtq(dtqid=%d, data=0x%x)\n",               \
            _kernel_pc(), _kernel_er(r), id, data);          \
    }
# define _KERNEL_ASSERT_TRCV_DTQ() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "trcv_dtq(dtqid=%d, p_data=0x%x, tmout=%d)\n",         \
            _kernel_pc(), _kernel_er(r), id, p_data, tmout);       \
    }
# define _KERNEL_ASSERT_REF_DTQ()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_dtq(dtqid=%d, pk_rdtq=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rdtq);       \
    }
# define _KERNEL_ASSERT_CRE_MBX()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                         \
            "cre_mbx(mbxid=%d, pk_cmbx=0x%x) "                  \
            "mbxatr=0x%x, maxmpri=%d, "                         \
            "mprihd=0x%x, name=%s\n",                           \
            _kernel_pc(), _kernel_er(r),                        \
            mbxid, pk_cmbx,                                     \
            pk_cmbx->mbxatr, pk_cmbx->maxmpri,                  \
            pk_cmbx->mprihd, pk_cmbx->name?pk_cmbx->name:"");   \
    }
# define _KERNEL_ASSERT_DEL_MBX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_mbx(mbxid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_SND_MBX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "snd_mbx(mbxid=%d, pk_msg=0x%x)\n",              \
            _kernel_pc(), _kernel_er(r), id, pk_msg);        \
    }
# define _KERNEL_ASSERT_TRCV_MBX() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "trcv_dtq(mbxid=%d, ppk_msg=0x%x, tmout=%d)\n",        \
            _kernel_pc(), _kernel_er(r), id, ppk_msg, tmout);      \
    }
# define _KERNEL_ASSERT_REF_MBX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_mbx(mbxid=%d, pk_rmbx=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rmbx);       \
    }
# define _KERNEL_ASSERT_CRE_MTX()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                         \
            "cre_mtx(mtxid=%d, pk_cmtx=0x%x) "                  \
            "mtxatr=0x%x, ceilpri=%d, "                         \
            "name=%s\n",                                        \
            _kernel_pc(), _kernel_er(r),                        \
            mtxid, pk_cmtx,                                     \
            pk_cmtx->mtxatr, pk_cmtx->ceilpri,                  \
            pk_cmtx->name?pk_cmtx->name:"");                    \
    }
# define _KERNEL_ASSERT_DEL_MTX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "       \
            "del_mtx(mtxid=%d)\n",            \
            _kernel_pc(), _kernel_er(r), id); \
    }
# define _KERNEL_ASSERT_UNL_MTX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "       \
            "unl_mtx(mtxid=%d)\n",            \
            _kernel_pc(), _kernel_er(r), id); \
    }
# define _KERNEL_ASSERT_TLOC_MTX() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tloc_mtx(mtxid=%d, tmout=%d)\n",                      \
            _kernel_pc(), _kernel_er(r), id, tmout);               \
    }
# define _KERNEL_ASSERT_REF_MTX()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_mtx(mtxid=%d, pk_rmtx=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rmtx);       \
    }
# define _KERNEL_ASSERT_CRE_MBF()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                         \
            "cre_mbf(mbfid=%d, pk_cmbf=0x%x) "                  \
            "mbfatr=0x%x, maxmsz=%d, "                          \
            "mbfsz=%d, mbf=0x%x, name=%s\n",                    \
            _kernel_pc(), _kernel_er(r),                        \
            mbfid, pk_cmbf,                                     \
            pk_cmbf->mbfatr, pk_cmbf->maxmsz, pk_cmbf->mbfsz,   \
            pk_cmbf->mbf, pk_cmbf->name?pk_cmbf->name:"");      \
    }
# define _KERNEL_ASSERT_DEL_MBF()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_mbf(mbfid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_TSND_MBF() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tsnd_mbf(mbfid=%d, msg=0x%x, msgsz=%d, tmout=%d)\n",  \
            _kernel_pc(), _kernel_er(r), id, msg, msgsz, tmout);   \
    }
# define _KERNEL_ASSERT_TRCV_MBF() \
    if (r < 0 && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                        \
            "trcv_mbf(mbfid=%d, msg=0x%x, tmout=%d)\n",        \
            _kernel_pc(), _kernel_er(r), id, msg, tmout);      \
    }
# define _KERNEL_ASSERT_REF_MBF()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_mbf(mbfid=%d, pk_rmbf=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rmbf);       \
    }
# define _KERNEL_ASSERT_CRE_POR()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                          \
            "cre_por(porid=%d, pk_cpor=0x%x) "                   \
            "poratr=0x%x, maxcmsz=%d, maxrmsz=%d, "              \
            "name=%s\n",                                         \
            _kernel_pc(), _kernel_er(r),                         \
            porid, pk_cpor,                                      \
            pk_cpor->poratr, pk_cpor->maxcmsz, pk_cpor->maxrmsz, \
            pk_cpor->name?pk_cpor->name:"");                     \
    }
# define _KERNEL_ASSERT_DEL_POR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_por(porid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_TCAL_POR() \
    if (r < 0 && r != E_TMOUT && r != E_RLWAI && r != E_DLT) {     \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tcal_por(porid=%d, calptn=0x%x, msg=0x%x, ",          \
            "cmsgsz=%d, tmout=%d)\n",                              \
            _kernel_pc(), _kernel_er(r),                           \
            id, calptn, msg,                                       \
            cmsgsz, tmout);                                        \
    }
# define _KERNEL_ASSERT_TACP_POR() \
    if (r < 0 && r != E_TMOUT && r != E_RLWAI && r != E_DLT) {     \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tacp_por(porid=%d, acpptn=0x%x, p_rdvno=0x%x, "       \
            "msg=0x%x, tmout=%d)\n",                               \
            _kernel_pc(), _kernel_er(r),                           \
            id, acpptn, p_rdvno,                                   \
            msg, tmout);                                           \
    }
# define _KERNEL_ASSERT_FWD_POR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                            \
            "fwd_por(porid=%d, calptn=0x%x, rdvno=0x%x, "          \
            "msg=0x%x, cmsgsz=%d)\n",                              \
            _kernel_pc(), _kernel_er(r),                           \
            id, calptn, rdvno,                                     \
            msg, cmsgsz);                                          \
    }
# define _KERNEL_ASSERT_RPL_RDV()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                            \
            "rpl_rdv(rdvno=0x%x, msg=0x%x, rmsgsz=%d)\n",          \
            _kernel_pc(), _kernel_er(r),                           \
            rdvno, msg, rmsgsz);                                   \
    }
# define _KERNEL_ASSERT_REF_POR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_por(porid=%d, pk_rpor=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rpor);       \
    }
# define _KERNEL_ASSERT_REF_RDV()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_rdv(rdvno=0x%x, pk_rrdv=0x%x)\n",           \
            _kernel_pc(), _kernel_er(r), rdvno, pk_rrdv);    \
    }
# define _KERNEL_ASSERT_DEF_INH() \
    if (r != E_OK) {              \
        _kernel_assert("pc=0x%x er=%s "                                  \
            "def_inh(inhno=%d, pk_dinh=0x%x) inthdr=0x%x, imask=0x%x\n", \
            _kernel_pc(), _kernel_er(r),                                 \
            inhno, pk_dinh, pk_dinh->inthdr, pk_dinh->imask);            \
    }
# define _KERNEL_ASSERT_CHG_IMS()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "          \
            "chg_ims(imask=%d)\n",               \
            _kernel_pc(), _kernel_er(r), imask); \
    }
# define _KERNEL_ASSERT_GET_IMS()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "            \
            "get_ims(p_imask=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), p_imask); \
    }
# define _KERNEL_ASSERT_CRE_ISR() \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                                    \
            "cre_isr(isrid=%d, pk_cisr=0x%x) "                             \
            "exinf=0x%x, intno=%d, isr=0x%x, imask=0x%x\n",                \
            _kernel_pc(), _kernel_er(r),                                   \
            isrid, pk_cisr,                                                \
            pk_cisr->exinf, pk_cisr->intno, pk_cisr->isr, pk_cisr->imask); \
    }
# define _KERNEL_ASSERT_DEL_ISR() \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_isr(isrid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), isrid);             \
    }
# define _KERNEL_ASSERT_REF_ISR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_isr(isrid=%d, pk_risr=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), isrid, pk_risr);    \
    }

# define _KERNEL_ASSERT_CRE_MPL()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                          \
            "cre_mpl(mplid=%d, pk_cmpl=0x%x) "                   \
            "mplatr=0x%x, mplsz=%d, mpl=0x%x, "                  \
            "name=%s\n",                                         \
            _kernel_pc(), _kernel_er(r),                         \
            mplid, pk_cmpl,                                      \
            pk_cmpl->mplatr, pk_cmpl->mplsz, pk_cmpl->mpl,       \
            pk_cmpl->name?pk_cmpl->name:"");                     \
    }
# define _KERNEL_ASSERT_DEL_MPL()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_mpl(mplid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_TGET_MPL() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tsnd_mpl(mplid=%d, blksz=%d, p_blk=0x%x, tmout=%d)\n",\
            _kernel_pc(), _kernel_er(r), id, blksz, p_blk, tmout); \
    }
# define _KERNEL_ASSERT_REL_MPL()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "rel_mpl(mplid=%d, blk=0x%x)\n",                 \
            _kernel_pc(), _kernel_er(r), id, blk);           \
    }
# define _KERNEL_ASSERT_REF_MPL()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_mpl(mplid=%d, pk_rmpl=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rmpl);       \
    }
# define _KERNEL_ASSERT_CRE_MPF()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                          \
            "cre_mpf(mpfid=%d, pk_cmpf=0x%x) "                   \
            "mpfatr=0x%x, blkcnt=%d, blksz=%d, "                 \
            "mpf=0x%x, name=%s\n",                               \
            _kernel_pc(), _kernel_er(r),                         \
            mpfid, pk_cmpf,                                      \
            pk_cmpf->mpfatr, pk_cmpf->blkcnt, pk_cmpf->blksz,    \
            pk_cmpf->mpf, pk_cmpf->name?pk_cmpf->name:"");       \
    }
# define _KERNEL_ASSERT_DEL_MPF()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_mpf(mpfid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_TGET_MPF() \
    if (r != E_OK && r != E_TMOUT && r != E_RLWAI && r != E_DLT) { \
        _kernel_assert("pc=0x%x er=%s "                            \
            "tget_mpf(mpfid=%d, p_blf=0x%x, tmout=%d)\n",          \
            _kernel_pc(), _kernel_er(r), id, p_blf, tmout);        \
    }
# define _KERNEL_ASSERT_REL_MPF()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "rel_mpf(mpfid=%d, blf=0x%x)\n",                 \
            _kernel_pc(), _kernel_er(r), id, blf);           \
    }
# define _KERNEL_ASSERT_REF_MPF()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_mpf(mpfid=%d, pk_rmpf=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rmpf);       \
    }
# define _KERNEL_ASSERT_SET_TIM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "set_tim(p_systim=0x%x)\n",                      \
            _kernel_pc(), _kernel_er(r), p_systim);          \
    }
# define _KERNEL_ASSERT_GET_TIM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "get_tim(p_systim=0x%x)\n",                      \
            _kernel_pc(), _kernel_er(r), p_systim);          \
    }
# define _KERNEL_ASSERT_CRE_CYC()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "cre_cyc(cycid=%d, pk_ccyc=0x%x) "               \
            "cycatr=0x%x, exinf=0x%x, cychdr=0x%x, "         \
            "cyctim=%d, cycphs=%d, name=%s\n",               \
            _kernel_pc(), _kernel_er(r),                     \
            cycid, pk_ccyc,                                  \
            pk_ccyc->cycatr, pk_ccyc->exinf, pk_ccyc->cychdr,\
            pk_ccyc->cyctim, pk_ccyc->cycphs,                \
            pk_ccyc->name?pk_ccyc->name:"");                 \
    }
# define _KERNEL_ASSERT_DEL_CYC()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_cyc(cycid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_STA_CYC()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sta_cyc(cycid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_STP_CYC()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "stp_cyc(cycid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_REF_CYC()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_cyc(cycid=%d, pk_rcyc=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_rcyc);       \
    }
# define _KERNEL_ASSERT_CRE_ALM()  \
    if (r < 0) {                   \
        _kernel_assert("pc=0x%x er=%s "                      \
            "cre_alm(almid=%d, pk_calm=0x%x) "               \
            "almatr=0x%x, exinf=0x%x, almhdr=0x%x, "         \
            "name=%s\n",                                     \
            _kernel_pc(), _kernel_er(r),                     \
            almid, pk_calm,                                  \
            pk_calm->almatr, pk_calm->exinf, pk_calm->almhdr,\
            pk_calm->name?pk_calm->name:"");                 \
    }
# define _KERNEL_ASSERT_DEL_ALM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "del_alm(almid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_STA_ALM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sta_alm(almid=%d, almtim=%d)\n",                \
            _kernel_pc(), _kernel_er(r), id, almtim);        \
    }
# define _KERNEL_ASSERT_STP_ALM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "stp_alm(almid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), id);                \
    }
# define _KERNEL_ASSERT_REF_ALM()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_alm(almid=%d, pk_ralm=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), id, pk_ralm);       \
    }
# define _KERNEL_ASSERT_DEF_OVR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "def_ovr(pk_dovr=0x%x) "                         \
            "ovratr=0x%x, ovrhdr=0x%x\n",                    \
            _kernel_pc(), _kernel_er(r),                     \
            pk_dovr, pk_dovr->ovratr, pk_dovr->ovrhdr);      \
    }
# define _KERNEL_ASSERT_STA_OVR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sta_ovr(tskid=%d, ovrtim=%d)\n",                \
            _kernel_pc(), _kernel_er(r), tskid, ovrtim);     \
    }
# define _KERNEL_ASSERT_STP_OVR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "stp_ovr(tskid=%d)\n",                           \
            _kernel_pc(), _kernel_er(r), tskid);             \
    }
# define _KERNEL_ASSERT_REF_OVR()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_ovr(tskid=%d, pk_rovr=0x%x)\n",             \
            _kernel_pc(), _kernel_er(r), tskid, pk_rovr);    \
    }
# define _KERNEL_ASSERT_DEF_SVC()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "def_svc(fncd=%d, pk_dsvc=0x%x)\n",              \
            _kernel_pc(), _kernel_er(r), fncd, pk_dsvc);     \
    }
# define _KERNEL_ASSERT_ROT_RDQ()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "rot_rdq(tskpri=%d)\n",                          \
            _kernel_pc(), _kernel_er(r), tskpri);            \
    }
# define _KERNEL_ASSERT_GET_TID()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "get_tid(p_tskid=0x%x)\n",                       \
            _kernel_pc(), _kernel_er(r), p_tskid);           \
    }
# define _KERNEL_ASSERT_DIS_DSP()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "dis_dsp()\n",                                   \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_ENA_DSP()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ena_dsp()\n",                                   \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_REF_SYS()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_sys(pk_rsys=0x%x)\n",                       \
            _kernel_pc(), _kernel_er(r), pk_rsys);           \
    }

# define _KERNEL_ASSERT_REF_VER()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_ver(pk_rver=0x%x)\n",                       \
            _kernel_pc(), _kernel_er(r), pk_rver);           \
    }
# define _KERNEL_ASSERT_REF_CFG()  \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "ref_cfg(pk_rcfg=0x%x)\n",                       \
            _kernel_pc(), _kernel_er(r), pk_rcfg);           \
    }

# define _KERNEL_ASSERT_SYSINI()   \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "sysini()\n",                                    \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_SYSSTA()   \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "syssta()\n",                                    \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_INTSTA()   \
    if (r < 0) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "intsta()\n",                                    \
            _kernel_pc(), _kernel_er(r));                    \
    }
# define _KERNEL_ASSERT_INTEXT()   \
    if (r != E_OK) {               \
        _kernel_assert("pc=0x%x er=%s "                      \
            "intext()\n",                                    \
            _kernel_pc(), _kernel_er(r));                    \
    }

#else /* _KERNEL_ASSERT */
# define _KERNEL_ASSERT_CRE_TSK() {}
# define _KERNEL_ASSERT_DEL_TSK() {}
# define _KERNEL_ASSERT_ACT_TSK() {}
# define _KERNEL_ASSERT_CAN_ACT() {}
# define _KERNEL_ASSERT_STA_TSK() {}
# define _KERNEL_ASSERT_EXT_TSK() {}
# define _KERNEL_ASSERT_EXD_TSK() {}
# define _KERNEL_ASSERT_TER_TSK() {}
# define _KERNEL_ASSERT_CHG_PRI() {}
# define _KERNEL_ASSERT_GET_PRI() {}
# define _KERNEL_ASSERT_REF_TSK() {}
# define _KERNEL_ASSERT_REF_TST() {}
# define _KERNEL_ASSERT_SUS_TSK() {}

# define _KERNEL_ASSERT_RSM_TSK() {}
# define _KERNEL_ASSERT_FRSM_TSK() {}
# define _KERNEL_ASSERT_TSLP_TSK() {}
# define _KERNEL_ASSERT_WUP_TSK() {}
# define _KERNEL_ASSERT_CAN_WUP() {}
# define _KERNEL_ASSERT_REL_WAI() {}
# define _KERNEL_ASSERT_DLY_TSK() {}

# define _KERNEL_ASSERT_DEF_TEX() {}
# define _KERNEL_ASSERT_RAS_TEX() {}
# define _KERNEL_ASSERT_DIS_TEX() {}
# define _KERNEL_ASSERT_ENA_TEX() {}
# define _KERNEL_ASSERT_REF_TEX() {}

# define _KERNEL_ASSERT_CRE_SEM() {}
# define _KERNEL_ASSERT_DEL_SEM() {}
# define _KERNEL_ASSERT_SIG_SEM() {}
# define _KERNEL_ASSERT_TWAI_SEM() {}
# define _KERNEL_ASSERT_REF_SEM() {}

# define _KERNEL_ASSERT_CRE_FLG() {}
# define _KERNEL_ASSERT_DEL_FLG() {}
# define _KERNEL_ASSERT_SET_FLG() {}
# define _KERNEL_ASSERT_CLR_FLG() {}
# define _KERNEL_ASSERT_TWAI_FLG() {}
# define _KERNEL_ASSERT_REF_FLG() {}

# define _KERNEL_ASSERT_CRE_DTQ() {}
# define _KERNEL_ASSERT_DEL_DTQ() {}
# define _KERNEL_ASSERT_TSND_DTQ() {}
# define _KERNEL_ASSERT_FSND_DTQ() {}
# define _KERNEL_ASSERT_TRCV_DTQ() {}
# define _KERNEL_ASSERT_REF_DTQ() {}

# define _KERNEL_ASSERT_CRE_MBX() {}
# define _KERNEL_ASSERT_DEL_MBX() {}
# define _KERNEL_ASSERT_SND_MBX() {}
# define _KERNEL_ASSERT_TRCV_MBX() {}
# define _KERNEL_ASSERT_REF_MBX() {}

# define _KERNEL_ASSERT_CRE_MTX() {}
# define _KERNEL_ASSERT_DEL_MTX() {}
# define _KERNEL_ASSERT_UNL_MTX() {}
# define _KERNEL_ASSERT_TLOC_MTX() {}
# define _KERNEL_ASSERT_REF_MTX() {}

# define _KERNEL_ASSERT_CRE_MBF() {}
# define _KERNEL_ASSERT_DEL_MBF() {}
# define _KERNEL_ASSERT_TSND_MBF() {}
# define _KERNEL_ASSERT_TRCV_MBF() {}
# define _KERNEL_ASSERT_REF_MBF() {}

# define _KERNEL_ASSERT_CRE_POR() {}
# define _KERNEL_ASSERT_DEL_POR() {}
# define _KERNEL_ASSERT_TCAL_POR() {}
# define _KERNEL_ASSERT_TACP_POR() {}
# define _KERNEL_ASSERT_FWD_POR() {}
# define _KERNEL_ASSERT_RPL_RDV() {}
# define _KERNEL_ASSERT_REF_POR() {}
# define _KERNEL_ASSERT_REF_RDV() {}

# define _KERNEL_ASSERT_DEF_INH() {}
# define _KERNEL_ASSERT_CHG_IMS() {}
# define _KERNEL_ASSERT_GET_IMS() {}
# define _KERNEL_ASSERT_CRE_ISR() {}
# define _KERNEL_ASSERT_DEL_ISR() {}
# define _KERNEL_ASSERT_REF_ISR() {}

# define _KERNEL_ASSERT_CRE_MPL() {}
# define _KERNEL_ASSERT_DEL_MPL() {}
# define _KERNEL_ASSERT_TGET_MPL() {}
# define _KERNEL_ASSERT_REL_MPL() {}
# define _KERNEL_ASSERT_REF_MPL() {}

# define _KERNEL_ASSERT_CRE_MPF() {}
# define _KERNEL_ASSERT_DEL_MPF() {}
# define _KERNEL_ASSERT_TGET_MPF() {}
# define _KERNEL_ASSERT_REL_MPF() {}
# define _KERNEL_ASSERT_REF_MPF() {}

# define _KERNEL_ASSERT_SET_TIM() {}
# define _KERNEL_ASSERT_GET_TIM() {}

# define _KERNEL_ASSERT_CRE_CYC() {}
# define _KERNEL_ASSERT_DEL_CYC() {}
# define _KERNEL_ASSERT_STA_CYC() {}
# define _KERNEL_ASSERT_STP_CYC() {}
# define _KERNEL_ASSERT_REF_CYC() {}

# define _KERNEL_ASSERT_CRE_ALM() {}
# define _KERNEL_ASSERT_DEL_ALM() {}
# define _KERNEL_ASSERT_STA_ALM() {}
# define _KERNEL_ASSERT_STP_ALM() {}
# define _KERNEL_ASSERT_REF_ALM() {}

# define _KERNEL_ASSERT_DEF_OVR() {}
# define _KERNEL_ASSERT_STA_OVR() {}
# define _KERNEL_ASSERT_STP_OVR() {}
# define _KERNEL_ASSERT_REF_OVR() {}

# define _KERNEL_ASSERT_DEF_SVC() {}

# define _KERNEL_ASSERT_ROT_RDQ() {}
# define _KERNEL_ASSERT_GET_TID() {}
# define _KERNEL_ASSERT_DIS_DSP() {}
# define _KERNEL_ASSERT_ENA_DSP() {}
# define _KERNEL_ASSERT_REF_SYS() {}

# define _KERNEL_ASSERT_REF_VER() {}
# define _KERNEL_ASSERT_REF_CFG() {}

# define _KERNEL_ASSERT_SYSINI() {}
# define _KERNEL_ASSERT_SYSSTA() {}
# define _KERNEL_ASSERT_INTSTA() {}
# define _KERNEL_ASSERT_INTEXT() {}

#endif /* _KERNEL_ASSERT */

#endif /* _KERNEL_H_ */
