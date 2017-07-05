/*
 * common.c : Common kernel functions
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

#include "kernel.h"

volatile T_TCB *_kernel_cur _KERNEL_DATA = 0; /* current task TCB */
volatile UB _kernel_sts = T_KER_SYS;          /* kernel status */
volatile UB _kernel_dly _KERNEL_DATA = 0;     /* 1:delayed dispatch request */
volatile UB _kernel_tic _KERNEL_DATA = 0;     /* kernel tick cnt */
volatile TMO _kernel_sysclk _KERNEL_DATA = 0; /* kernel system clock */
volatile SYSTIM _kernel_systim _KERNEL_DATA = {0,0}; /* system time */
T_KERBLK_HEAD *_kernel_sysmem;                /* system memory pool header */
T_KERBLK_HEAD *_kernel_mplmem;                /* mpl memory pool header */
T_KERBLK_HEAD _kernel_stkmem;                 /* stack memory pool header */
char *_kernel_int_sp;                         /* interrupt handler initial sp */
char *_kernel_tim_sp;                         /* system timer task initial sp */
ER SYSER _KERNEL_DATA = 0;                    /* system call error code log */
UB _kernel_inest _KERNEL_DATA = 0;            /* interrupt nest counter */
B _kernel_rdvseq _KERNEL_DATA = 0;            /* rendezvous sequential number */
T_LNK _kernel_dmy_que;                        /* dummy waiting queue */
volatile OVRTIM *_kernel_ovrcnt _KERNEL_DATA = NULL; /* overrun counter */
volatile B _kernel_ovrreq _KERNEL_DATA = 0;   /* request overrun handler */
volatile FP _kernel_ovrhdr _KERNEL_DATA = NULL; /* overrun handler address */
B (*_kernel_chg_mtx_fp)(T_TCB *tcb, PRI pri); /* _kernel_chg_mtx() hook */
B (*_kernel_chk_wmbf_fp)(T_MBF *mbf);         /* _kernel_chk_wmbf() hook */
B (*_kernel_chk_wmpl_fp)(T_MPL *mpl);         /* _kernel_chk_wmpl() hook */
B (*_kernel_unl_amtx_fp)(T_TCB *atcb);        /* _kernel_unl_amtx() hook */

#ifdef _KERNEL_MTX
B *_kernel_inherit;              /* task inherit map for mutex */
#endif /* _KERNEL_MTX */

#ifdef _KERNEL_ASSERT
/*
 * Error code name (for assertion)
 */
char *_kernel_er(ER er)
{
    switch(er) {
    case E_OK   : return "E_OK   ";
    case E_SYS  : return "E_SYS  ";
    case E_NOSPT: return "E_NOSPT";
    case E_RSFN : return "E_RSFN ";
    case E_RSATR: return "E_RSATR";
    case E_PAR  : return "E_PAR  ";
    case E_ID   : return "E_ID   ";
    case E_CTX  : return "E_CTX  ";
    case E_MACV : return "E_MACV ";
    case E_OACV : return "E_OACV ";
    case E_ILUSE: return "E_ILUSE";
    case E_NOMEM: return "E_NOMEM";
    case E_NOID : return "E_NOID ";
    case E_OBJ  : return "E_OBJ  ";
    case E_NOEXS: return "E_NOEXS";
    case E_QOVR : return "E_QOVR ";
    case E_RLWAI: return "E_RLWAI";
    case E_TMOUT: return "E_TMOUT";
    case E_DLT  : return "E_DLT  ";
    case E_CLS  : return "E_CLS  ";
    case E_WBLK : return "E_WBLK ";
    case E_BOVR : return "E_BOVR ";
    default:      return "UNKNOWN";
    }
}
#endif /* _KERNEL_ASSERT */

/*****************************************
 * bi-directional linked queue functions *
 *****************************************/

/*
 * init queue(s)
 *   pri: max number of priority
 */
void _kernel_nul_que(T_LNK *que, PRI pri)
{
    for (; pri--; que++)
        que->p = que->n = que;
}

/*
 * add element to tail of the queue
 */
void _kernel_enq(T_LNK *que, T_LNK *add)
{
    T_LNK *end;
    
    if(add->n)                  /* already linked */
        _KERNEL_PANIC();
    add->n = que;
    end = que->p;
    add->p = end;
    end->n = add;
    que->p = add;
}

/*
 * remove element from the queue
 */
void _kernel_deq(T_LNK *del)
{
    T_LNK *p = del->p;
    T_LNK *n = del->n;
#ifdef _KERNEL_DEBUG
    if (!n)
        _KERNEL_PANIC();
#endif
    p->n = n;
    n->p = p;
    del->n = NULL;
}

/*
 * get first priority element from priority queue
 *     que : first queue
 *     pri : count of priority queue (1 means no priority)
 */
T_LNK *_kernel_first(T_LNK *que, PRI pri)
{
    for (; pri; pri--, que++) {
        if (que->n != que)
            return que->n;
    }
    return NULL;
}

/*
 * Force dispatch the task
 */
void _kernel_run(T_TCB *new)
{
    if (_KERNEL_CHK_ISLD(_kernel_sts)) {
        _kernel_dly = 1;        /* delayed dispatch */
    } else {
        _KERNEL_SW(new);        /* dispatch */
    }
}

/*
 * Dispatch if the task is higher than current
 */
void _kernel_higher(T_TCB *new)
{
    T_TCB *cur = (T_TCB *)_kernel_cur; /* avoid volatile effect */
    if (!cur || new->pri < cur->pri) { /* higher ? */
        if (_KERNEL_CHK_ISLD(_kernel_sts)) {
            _kernel_dly = 1;    /* delayed dispatch */
        } else {
            _KERNEL_SW(new);    /* dispatch */
        }
    }
}

/*
 * Dispatch highest task
 */
void _kernel_highest(void)
{
    T_LNK *rdq;
    
    if (_KERNEL_CHK_ISLD(_kernel_sts)) {
        _kernel_dly = 1;        /* delayed dispatch */
    } else {
        rdq = &_kernel_rdq[0];
        while (rdq->n == rdq)
            rdq++;
        _KERNEL_SW((T_TCB *)rdq->n); /* dispatch */
    }
}

/*
 * make current task waiting and dispatch other task
 *   tcb   : current task tcb
 *   tmout : time out value (or TMO_FEVR)
 *   que   : waiting queue
 *   pri   : 0 = no priority single queue, not 0 = multi priority queue
 *   return value : received return value through TCB
 */
ER _kernel_wai(T_TCB *tcb, TMO tmout, T_LNK *que, BOOL pri)
{
    _kernel_deq((T_LNK *)tcb);
    tcb->sts = TTS_WAI;
    if (tmout != TMO_FEVR) {
        T_TIM *tim = &tcb->tim;
#ifdef _KERNEL_N4
        tim->tmo = _kernel_sysclk + tmout;
#else
        tim->tmo = _kernel_sysclk + tmout + 1;
#endif
        _kernel_enq_tmq(&_kernel_tmrq, tim);
    }
    if (pri)
        que += (tcb->pri - 1);
    _kernel_enq(que, (T_LNK *)tcb);
    _kernel_highest();          /* dispatch */

    return _KERNEL_GET_TCB_RET(tcb);
}

/*
 * 1) unlink timer block
 * 2) unlink from any queue
 * 3) change task status 
 *        WAITING -> READY
 *        WAITING-SUSPENDED -> SUSPENDED
 * 4) if READY then dispatch higher
 */
void _kernel_deq_rdy_dsp(T_TCB *tcb)
{
#ifdef _KERNEL_DEBUG
    if (!(tcb->sts & TTS_WAI))
        _KERNEL_PANIC();
#endif
    if (tcb->tim.n)
        _kernel_deq((T_LNK *)&tcb->tim); /* remove from timer queue */
    _kernel_deq((T_LNK *)tcb);
    if (tcb->sts == TTS_WAI) {
        tcb->sts = TTS_RDY;
        _kernel_enq(&_kernel_rdq[tcb->pri - 1], (T_LNK *)tcb);
        _kernel_higher(tcb);    /* dispatch */
    } else {                    /* TTS_WAS */
        tcb->sts = TTS_SUS;
    }
}

/*
 * 1) unlink timer block
 * 2) unlink from any queue
 * 3) change task status 
 *        WAITING -> READY (return 1)
 *        WAITING-SUSPENDED -> SUSPENDED (return 0)
 */
B _kernel_deq_rdy(T_TCB *tcb)
{
#ifdef _KERNEL_DEBUG
    if (!(tcb->sts & TTS_WAI))
        _KERNEL_PANIC();
#endif
    if (tcb->tim.n)
        _kernel_deq((T_LNK *)&tcb->tim); /* remove from timer queue */
    _kernel_deq((T_LNK *)tcb);
    if (tcb->sts == TTS_WAI) {
        tcb->sts = TTS_RDY;
        _kernel_enq(&_kernel_rdq[tcb->pri - 1], (T_LNK *)tcb);
        return 1;
    } else {                    /* TTS_WAS */
        tcb->sts = TTS_SUS;
        return 0;
    }
}

/*
 * remove TCB from waiting queue and make READY or SUSPENDED
 */
void _kernel_dlt(T_LNK *que, PRI pri)
{
    for (; pri; pri--, que++) {
        T_TCB *tcb = (T_TCB *)que->n;
        while (tcb != (T_TCB *)que) {
            /*
             * remove from waiting queue
             */
            T_TCB *tcb_n = tcb->n;
            _KERNEL_SET_TCB_RET(tcb, E_DLT);
            _kernel_deq_rdy(tcb);
            tcb = tcb_n;
        }
    }
}

/*
 * Make task acitive and dispatch
 */
void _kernel_act(T_TCB *tcb, VP_INT arg)
{
    T_TCB *cur = (T_TCB *)_kernel_cur; /* avoid volatile effect */

    /* 
     * initialize task context, make READY and dispatch
     */
    _KERNEL_INI_CTX();
#ifdef _KERNEL_MTX
    tcb->bpri = tcb->pri = tcb->ctsk->itskpri;
#else
    tcb->pri = tcb->ctsk->itskpri;
#endif
    tcb->sts = TTS_RDY;
    tcb->wup = 0;
    tcb->sus = 0;
#ifdef _KERNEL_MTX
    tcb->mtx = 0;
#endif
    if (tcb->tex) {
        tcb->tex->ptn  = 0;
        tcb->tex->stat = TTEX_DIS;
    }
    _kernel_enq(&_kernel_rdq[tcb->pri - 1], (T_LNK *)tcb);
    /*
     * current RUNNING task
     */
    if (tcb == cur) {
        _kernel_cur = NULL;     /* purge context */
        if (_KERNEL_CHK_ISLD(_kernel_sts)) {
            _kernel_dly = 1;    /* delayed dispatch */
        } else {
            _KERNEL_SW(tcb);    /* dispatch */
        }
    }
    /*
     * another task
     */
    else {
        if (!cur || tcb->pri < cur->pri) { /* higher ? */
            if (_KERNEL_CHK_ISLD(_kernel_sts)) {
                _kernel_dly = 1; /* delayed dispatch */
            } else {
                _KERNEL_SW(tcb); /* dispatch */
            }
        }
    }
}

/*
 * Call task exception handler
 */
void _kernel_tex(T_TCB *tcb, UINT psw)
{
    T_TEX *tex = (T_TEX *)tcb->tex;
    if (!tex || tex->stat != TTEX_ENA || !tex->ptn)
        return;
    do {
        TEXPTN ptn = tex->ptn;
        FP_TEX rtn = (FP_TEX)tex->dtex->texrtn;
        tex->ptn = 0;
        tex->stat = TTEX_DIS;
        _KERNEL_SET_PSW(psw);
        (*rtn)(ptn, tcb->ctsk->exinf); /* handler runs on the task context */
        _KERNEL_DIS();
        if (!(tex = (T_TEX *)tcb->tex))
            return;         /* tex removed */
    } while (tex->ptn);
    tex->stat = TTEX_ENA;
}

/*
 * system timer task
 *     psw : saved CPU-PSW value by timer interrupt
 */
void _kernel_tim(UINT psw)
{
    UB tic;

    /*
     * timer task main loop
     */
    while ((tic = _kernel_tic) != 0) {
        TMO sysclk;
        _kernel_tic = tic - 1;
        /* 
         * update system clock and system time
         */
        sysclk = ++_kernel_sysclk;
        if (!++_kernel_systim.ltime)
            _kernel_systim.utime++;

#ifdef _KERNEL_OVR
        /*
         * check overrun handler
         */
        if (_kernel_ovrcnt && _kernel_ovrreq) {
            OVRTIM *cnt;
            ID id;
            _kernel_ovrreq = 0;
            cnt = (OVRTIM *)_kernel_ovrcnt;
            for (id = 1; id <= _kernel_tskid_max; id++, cnt++) {
                /*
                 * call handler with interrupt acceptance
                 */
                OVRTIM c = *cnt;
                if (!++c) {     /* if (*cnt == -1) */
                    *cnt = c;   /* *cnt = 0 */
                    _KERNEL_SET_PSW(psw);
                    (*_kernel_ovrhdr)(id, _kernel_tcb[id - 1]->ctsk->exinf);
                    _KERNEL_DIS();
                    if (!_kernel_ovrcnt)
                        break;  /* overrun handler deleted oneself */
                }
            }
        }
#endif /* _KERNEL_OVR */

        /*
         * check cyclic handler
         */
        {
            T_TIMQUE *tmq;
            T_LNK *que;
            T_TIM *tim;

            tmq = &_kernel_cycq;
            que = &tmq->que[sysclk & tmq->msk];
restart_cyc:
            tmq->rst = 0;
            tim = (T_TIM *)que->n;
            while (tim != (T_TIM *)que) {
                T_TIM *tim_n = tim->n;
                /*
                 * timed out : reload timer
                 */
                if (tim->tmo == sysclk) {
                    tim->tmo = sysclk + ((T_CYC *)tim)->ccyc->cyctim;
                    _kernel_deq_tmq(tmq, tim);
                    tmq->rst = 0; /* cancel _kernel_deq_tmq() effect */
                    _kernel_enq_tmq(tmq, tim);
                    /*
                     * call handler with interrupt acceptance
                     */
                    if (((T_CYC *)tim)->sta) {
                        _KERNEL_SET_PSW(psw);
                        (*((T_CYC *)tim)->ccyc->cychdr)(((T_CYC *)tim)->ccyc->exinf);
                        _KERNEL_DIS();
                    }
                }
                /*
                 * accept interrupt by loop
                 */
                else {
                    _KERNEL_SET_PSW(psw);
                    _KERNEL_INTGAP();
                    _KERNEL_DIS();
                }
                if (tmq->rst)
                    goto restart_cyc;
                tim = tim_n;
            }
        }
        /*
         * check alarm handler
         */
        {
            T_TIMQUE *tmq;
            T_LNK *que;
            T_TIM *tim;

            tmq = &_kernel_almq;
            que = &tmq->que[sysclk & tmq->msk];
restart_alm:
            tmq->rst = 0;
            tim = (T_TIM *)que->n;
            while (tim != (T_TIM *)que) {
                T_TIM *tim_n = tim->n;
                /*
                 * timed out : remove timer
                 */
                if (tim->tmo == sysclk) {
                    _kernel_deq_tmq(tmq, tim);
                    tmq->rst = 0; /* cancel _kernel_deq_tmq() effect */
                    /*
                     * call handler with interrupt acceptance
                     */
                    _KERNEL_SET_PSW(psw);
                    (*((T_ALM *)tim)->calm->almhdr)(((T_ALM *)tim)->calm->exinf);
                    _KERNEL_DIS();
                }
                /*
                 * accept interrupt by loop
                 */
                else {
                    _KERNEL_SET_PSW(psw);
                    _KERNEL_INTGAP();
                    _KERNEL_DIS();
                }
                if (tmq->rst)
                    goto restart_alm;
                tim = tim_n;
            }
        }
        /*
         * check task timer
         */
        {
            T_TIMQUE *tmq;
            T_LNK *que;
            T_TIM *tim;

            tmq = &_kernel_tmrq;
            que = &tmq->que[sysclk & tmq->msk];
restart_tmr:
            tmq->rst = 0;
            tim = (T_TIM *)que->n;
            while (tim != (T_TIM *)que) {
                T_TIM *tim_n = tim->n;
                /*
                 * timed out : remove timer and wake up
                 */
                if (tim->tmo == sysclk) {
                    T_TCB *tcb;
                    ER ret;
                    _kernel_deq_tmq(tmq, tim);
                    tmq->rst = 0; /* cancel _kernel_deq_tmq() effect */
                    tcb = TCNV_TIM2TCB(tim);
                    ret = (tcb->wai == TTW_DLY)? E_OK : E_TMOUT;
                    _KERNEL_SET_TCB_RET(tcb, ret);
                    /*
                     * request dispatch if higher than current task
                     */
                    if (_kernel_deq_rdy(tcb)) {
                        T_TCB *cur = (T_TCB *)_kernel_cur;
                        if (!cur || tcb->pri < cur->pri)
                            _kernel_dly = 1;
                    }
                }
                /*
                 * accept interrupt
                 */
                _KERNEL_SET_PSW(psw);
                _KERNEL_INTGAP();
                _KERNEL_DIS();
                if (tmq->rst)
                    goto restart_tmr;
                tim = tim_n;
            }
        }
    }
}
                                
/*
 * Initialize kernel memory pool
 */
void _kernel_ini_mem(T_KERBLK_HEAD *head, char *adr, SIZE siz)
{
    head->str = adr;
    siz &= ~(sizeof(VP) - 1);
    head->end = adr + siz;
    head->last = &head->zero;
    head->zero.next = (T_KERBLK *)adr;
    head->zero.len = 0;
    ((T_KERBLK *)adr)->next = &head->zero;
    ((T_KERBLK *)adr)->len  = siz;
}

/*
 * Alloc kernel memory
 *     return value :
 *         not 0 : memory block pointer
 *         NULL  : no memory
 */
char *_kernel_alloc(T_KERBLK_HEAD *head, SIZE siz)
{
    SIZE len;
    T_KERBLK *old, *new;

    if (!siz)
        return NULL;
    len = sizeof(T_KERBLK) + TSZ_PTR(siz);
    old = head->last;
    new = old->next;
    for (;;) {
        if (len <= new->len) {
            /*
             * divide large block
             */
            if (len + sizeof(T_KERBLK) + sizeof(VP) <= new->len) {
                new->len -= len;
                new = (T_KERBLK *)((B *)new + new->len);
                new->len = len;
            }
            /*
             * not divide
             */
            else {
                old->next = new->next;
            }
            new->next = NULL;   /* for _kernel_free() */
            (++new)->next = NULL; /* for _kernel_enq() */
            head->last = old;   /* update last access point */
            return (char *)new;
        }
        if (new == head->last)
            return NULL;        /* no more free block */
        old = new;
        new = new->next;
    }
}

/*
 * Free kernel memory
 *     return value :
 *         E_OK  : success
 *         E_PAR : bad address or bad block
 */
ER _kernel_free(T_KERBLK_HEAD *head, char *adr)
{
    T_KERBLK *rel, *chk;

    /*
     * check released block
     */
    rel = ((T_KERBLK *)adr) - 1;
    if ((char *)rel < head->str || head->end <= (char *)rel ||
        rel->next               || !rel->len                 )
        return E_PAR;
    /*
     * lookup connect point
     */
    chk = head->last;
    while (rel < chk || chk->next < rel) {
        if (chk->next <= chk && (chk < rel || rel < chk->next))
            break;              /* connect to either end */
        chk = chk->next;
    }
    /*
     * connect forward
     */
    if (chk->next == (T_KERBLK *)((B *)rel + rel->len)) {
        rel->len += chk->next->len;
        rel->next = chk->next->next;
    } else {
        rel->next = chk->next;
    }
    /*
     * connect backwoad
     */
    if ((T_KERBLK *)((B *)chk + chk->len) == rel) {
        chk->len += rel->len;
        chk->next = rel->next;
    } else {
        chk->next = rel;
    }
    head->last = chk;           /* update last access point */
    return E_OK;
}

char *_kernel_sys_alloc(SIZE siz)
{
    return _kernel_alloc(_kernel_sysmem, siz);
}

char *_kernel_mpl_alloc(SIZE siz)
{
    return _kernel_alloc(_kernel_mplmem, siz);
}

char *_kernel_stk_alloc(SIZE siz)
{
    return _kernel_alloc(&_kernel_stkmem, siz);
}

void _kernel_sys_free(VP mem)
{
    if (_kernel_free(_kernel_sysmem, (char *)mem) != E_OK)
        _KERNEL_PANIC();
}

void _kernel_mpl_free(VP mem)
{
    if (_kernel_free(_kernel_mplmem, (char *)mem) != E_OK)
        _KERNEL_PANIC();
}

void _kernel_stk_free(VP mem)
{
    if (_kernel_free(&_kernel_stkmem, (char *)mem) != E_OK)
        _KERNEL_PANIC();
}

/*
 * dummy for any hook
 */
B _kernel_dmy_hook()
{
    return 0;
}

/*
 * unkown interrupt handler
 */
static void unkown_int(INHNO irq)
{
#ifdef _KERNEL_DEBUG
    printf("\nKERNEL PANIC : unkown interrupt, irq = %d\n\n", irq);
#endif /* _KERNEL_DEBUG */
    for (;;) ;
}

/****************************************
 * mono-directional msg queue functions *
 ****************************************/

/*
 * init msg queue(s)
 *   pri: max number of priority
 */
void _kernel_nul_msg_que(T_MSGQUE *que, PRI pri)
{
    for (; pri; pri--, que++)
        que->end = que->str = (T_MSG *)que;
}

/*
 * add msg to tail of the msg queue by priority
 *   pri: msg priority (1 .. max priority)
 */
void _kernel_enq_msg(T_MSGQUE *que, T_MSG *msg, PRI pri)
{
    if (msg->next)              /* msg already linked */
        _KERNEL_PANIC();
    que += (pri - 1);
    msg->next = (T_MSG *)que;
    que->end->next = msg;
    que->end = msg;
}

/*
 * remove top from msg queue(s)
 *   pri: max number of priority
 */
T_MSG *_kernel_deq_msg(T_MSGQUE *que, PRI pri)
{
    T_MSG *msg;

    for(; pri; pri--, que++) {
        msg = que->str;
        if (msg != (T_MSG *)que) { /* found */
            que->str = msg->next;
            msg->next = NULL;   /* for error checking at _kernel_enq_msg() */
            if (que->end == msg)
                que->end = (T_MSG *)que;
            return msg;
        }
    }
    return NULL;                /* not found */
}

/*
 * get first priority msg from queue
 */
T_MSG *_kernel_first_msg(T_MSGQUE *que, PRI pri)
{
    for (; pri; pri--, que++) {
        if (que->str != (T_MSG *)que)
            return que->str;
    }
    return NULL;
}

/**************************
 * system functions (API) *
 **************************/

ER sysini(void)
{
    ER r = E_OK;
    
    /*
     * init kernel status
     */
    _KERNEL_INI_SUB();
    /*
     * init queues
     */
    /* ready queue */
    _kernel_nul_que(&_kernel_rdq[0], _kernel_tpri_max);
    /* dummy waiting queue */
    _kernel_nul_que(&_kernel_dmy_que, 1);
    /* init hashed queue for task timer */
    _kernel_nul_que(_kernel_tmrq.que, _kernel_tmrq.msk + 1);
    /* init hashed queue for CYC timer */
    _kernel_nul_que(_kernel_cycq.que, _kernel_cycq.msk + 1);
    /* init hashed queue for ALM timer */
    _kernel_nul_que(_kernel_almq.que, _kernel_almq.msk + 1);
    /* init hashed queue for ISR */
    _kernel_nul_que(&_kernel_isrq[0], _kernel_isrq_msk + 1);

#ifdef _KERNEL_NO_BSSCLR
    /*
     * clear vars (when C start up routine does not clear BSS section)
     */
    _kernel_rdq[_kernel_tpri_max].n = NULL; /* NULL : idle task */
    memset(&_kernel_tcb[0], 0, _kernel_tskid_max * sizeof(VP));
    memset(&_kernel_sem[0], 0, _kernel_semid_max * sizeof(VP));
    memset(&_kernel_flg[0], 0, _kernel_flgid_max * sizeof(VP));
    memset(&_kernel_dtq[0], 0, _kernel_dtqid_max * sizeof(VP));
    memset(&_kernel_mbx[0], 0, _kernel_mbxid_max * sizeof(VP));
    memset(&_kernel_mtx[0], 0, _kernel_mtxid_max * sizeof(VP));
    memset(&_kernel_mbf[0], 0, _kernel_mbfid_max * sizeof(VP));
    memset(&_kernel_por[0], 0, _kernel_porid_max * sizeof(VP));
    memset(&_kernel_isr[0], 0, _kernel_isrid_max * sizeof(VP));
    memset(&_kernel_mpl[0], 0, _kernel_mplid_max * sizeof(VP));
    memset(&_kernel_mpf[0], 0, _kernel_mpfid_max * sizeof(VP));
    memset(&_kernel_cyc[0], 0, _kernel_cycno_max * sizeof(VP));
    memset(&_kernel_alm[0], 0, _kernel_almno_max * sizeof(VP));
    memset(&_kernel_svc[0], 0, _kernel_svcfn_max * sizeof(VP));
#endif /* _KERNEL_NO_BSSCLR */

    /*
     * init hook
     */
    _kernel_chg_mtx_fp  = (B (*)(T_TCB*, PRI))_kernel_dmy_hook;
    _kernel_chk_wmbf_fp = (B (*)(T_MBF*))     _kernel_dmy_hook;
    _kernel_chk_wmpl_fp = (B (*)(T_MPL*))     _kernel_dmy_hook;
    _kernel_unl_amtx_fp = (B (*)(T_TCB*))     _kernel_dmy_hook;

    /*
     * alloc pool of stack memory from main() stack
     */
    {
        char *t = (char *)_KERNEL_ALIGN     (_KERNEL_STKTOP());
        char *b = (char *)_KERNEL_ALIGN_BACK(_KERNEL_STKBOT());
        SIZE sz = b - t;
        _kernel_ini_mem(&_kernel_stkmem, t, sz);
    }
    /*
     * alloc system timer task stack
     *   ###############################################
     *   ### Must be 1st use of _kernel_stk_alloc()  ###
     *   ### because to allocate at bottom of stack. ###
     *   ###############################################
     */
    {
        char *p;
        if (!(p = _kernel_stk_alloc(_kernel_tstksz)))
            _KERNEL_RET(E_NOMEM);
        _kernel_tim_sp = p + _kernel_tstksz;

#ifndef _KERNEL_NO_STKFIL
        /* system timer task stack fill pattern : 0xee */
        {
            char *sp = _KERNEL_GET_SP() - 16 /* safety margin */;
            SIZE sz = _kernel_tstksz;
            while (sz-- && p < sp)
                *p++ = 0xee;
        }
#endif

    }
    /*
     * alloc interrupt handler stack
     */
    {
        char *p;
        if (!(p = _kernel_stk_alloc(_kernel_istksz)))
            _KERNEL_RET(E_NOMEM);
        _kernel_int_sp = p + _kernel_istksz;

#ifndef _KERNEL_NO_STKFIL
        /* interrupt handler stack fill pattern : 0xdd */
        memset(p, 0xdd, _kernel_istksz);
#endif

    }
    /*
     * alloc the pool of system memory
     */
    if (!_kernel_sysmsz) {
        _kernel_sysmem = &_kernel_stkmem;
    } else {
        char *p;
        SIZE sz = sizeof(T_KERBLK_HEAD) + sizeof(T_KERBLK) + _kernel_sysmsz;
        if (!(p = _kernel_stk_alloc(sz)))
            _KERNEL_RET(E_SYS);
        _kernel_ini_mem(_kernel_sysmem = (T_KERBLK_HEAD *)p,
                        p + sizeof(T_KERBLK_HEAD),
                        sizeof(T_KERBLK) + _kernel_sysmsz);
    }
    /*
     * alloc the pool of memory pool memory
     */
    if (!_kernel_mplmsz) {
        _kernel_mplmem = &_kernel_stkmem;
    } else {
        char *p;
        SIZE sz = sizeof(T_KERBLK_HEAD) + sizeof(T_KERBLK) + _kernel_mplmsz;
        if (!(p = _kernel_stk_alloc(sz)))
            _KERNEL_RET(E_NOMEM);
        _kernel_ini_mem(_kernel_mplmem = (T_KERBLK_HEAD *)p,
                        p + sizeof(T_KERBLK_HEAD),
                        sizeof(T_KERBLK) + _kernel_mplmsz);
    }

#ifdef _KERNEL_MTX
    /*
     * alloc task inherit map
     */
    if (!(_kernel_inherit = (B *)_kernel_stk_alloc(_kernel_tskid_max)))
        _KERNEL_RET(E_NOMEM);
#endif /* _KERNEL_MTX */

    /*
     * set pesudo interrupt vectors to default handler
     */
    {
        INHNO irq;
        T_VECT *vec = &_kernel_vect[0];
        for (irq = 0; irq < _KERNEL_IRQ_CNT; irq++, vec++) {
            vec->inthdr = (FP_INH)unkown_int;
# ifdef _KERNEL_INT_IMASK
            vec->imask  = _KERNEL_LVL2PSW(_kernel_level);
# endif
        }
    }

    /*
     * check STKMSZ
     */
    if (_kernel_stkmsz &&
        _kernel_stkmem.zero.next->len < _kernel_stkmsz)
        _KERNEL_RET(E_NOMEM);
ret:
    _KERNEL_ASSERT_SYSINI();
    return r;
}

ER syssta(void)
{
    ER r = SYSER;
    if (!r) {
        _kernel_sts = 0;
        _kernel_highest();      /* start dispatch and never return */
    }
    _KERNEL_ASSERT_SYSSTA();
    return r;
}

/* end */
