#
# Makefie : Makefile sample for H8/300H
#
# Copyright (C) 2007,2008,2009,2010
# 256TECH Co., Ltd.
# Masahiro Sakamoto (m-sakamoto@users.sourceforge.net)
#
# This file is part of URIBO.
#
# URIBO is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# URIBO is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with URIBO; see the file COPYING and COPYING.LESSER.
# If not, see <http://www.gnu.org/licenses/>.
#

VPATH = ../:../../kernel
TARGET = test.elf

CC  = h8300-linux-elf-gcc
AR  = h8300-linux-elf-ar
NM  = h8300-linux-elf-nm
DMP = h8300-linux-elf-objdump

#
# <kernel compile options>
#   H8_3069        : select device register header for H8/3069
#   _KERNEL_ASSERT : enable system call assertion
#
CFLAGS = -DH8_3069 -D_KERNEL_ASSERT -ggdb -O0 -mh -mint32 -malign-300 -fsigned-char -I../ -I../../kernel

NMFLAGS  = -n
DMPFLAGS = -S

.SUFFIXES: .elf

.S.o:	$(CC) -c $(CFLAGS) $<

.c.o:	$(CC) -c $(CFLAGS) $<

#
# KOBJS : kernel objects
#
KOBJS= \
common.o \
cre_tsk.o \
del_tsk.o \
act_tsk.o \
can_act.o \
sta_tsk.o \
ext_tsk.o \
exd_tsk.o \
ter_tsk.o \
chg_pri.o \
get_pri.o \
ref_tsk.o \
ref_tst.o \
\
sus_tsk.o \
rsm_tsk.o \
frsm_tsk.o \
tslp_tsk.o \
wup_tsk.o \
can_wup.o \
rel_wai.o \
dly_tsk.o \
\
def_tex.o \
ras_tex.o \
dis_tex.o \
ena_tex.o \
sns_tex.o \
ref_tex.o \
\
cre_sem.o \
del_sem.o \
sig_sem.o \
twai_sem.o \
ref_sem.o \
\
cre_flg.o \
del_flg.o \
set_flg.o \
clr_flg.o \
twai_flg.o \
ref_flg.o \
\
cre_dtq.o \
del_dtq.o \
tsnd_dtq.o \
fsnd_dtq.o \
trcv_dtq.o \
ref_dtq.o \
\
cre_mbx.o \
del_mbx.o \
snd_mbx.o \
trcv_mbx.o \
ref_mbx.o \
\
cre_mtx.o \
del_mtx.o \
unl_mtx.o \
tloc_mtx.o \
ref_mtx.o \
\
cre_mbf.o \
del_mbf.o \
tsnd_mbf.o \
trcv_mbf.o \
ref_mbf.o \
\
cre_por.o \
del_por.o \
tcal_por.o \
tacp_por.o \
fwd_por.o \
rpl_rdv.o \
ref_por.o \
ref_rdv.o \
\
def_inh.o \
chg_ims.o \
get_ims.o \
cre_isr.o \
del_isr.o \
ref_isr.o \
\
cre_mpl.o \
del_mpl.o \
tget_mpl.o \
rel_mpl.o \
ref_mpl.o \
\
cre_mpf.o \
del_mpf.o \
tget_mpf.o \
rel_mpf.o \
ref_mpf.o \
\
set_tim.o \
get_tim.o \
\
cre_cyc.o \
del_cyc.o \
sta_cyc.o \
stp_cyc.o \
ref_cyc.o \
\
cre_alm.o \
del_alm.o \
sta_alm.o \
stp_alm.o \
ref_alm.o \
\
def_ovr.o \
sta_ovr.o \
stp_ovr.o \
ref_ovr.o \
\
def_svc.o \
\
rot_rdq.o \
get_tid.o \
dis_dsp.o \
ena_dsp.o \
ref_sys.o \
\
ref_ver.o \
ref_cfg.o

#
# DOBJS : device dependence objects
#
DOBJS = sw_h8300h.o timer.o tprintf.o sci.o

#
# AOBJS : user application objects
#
AOBJS = main.o task1.o task2.o

all: $(TARGET)

$(TARGET): itron.h kernel.h config.h itron_s.h kernel_s.h config_s.h\
 ramcrt0.o $(AOBJS) $(DOBJS) libkernel.a
	$(CC) $(CFLAGS) -nostartfiles -Tram.x -o $(TARGET) -Wl,-Map,$*.lmap ramcrt0.o $(AOBJS) $(DOBJS) libkernel.a
	$(NM) $(NMFLAGS) $@ > $*.map
	$(DMP) $(DMPFLAGS) $@ > $*.dmp

libkernel.a: $(KOBJS)
	$(AR) rcs $@ $(KOBJS)

clean:
	rm -f *.o *.a *.dmp *.map *.lmap *.elf

#end
