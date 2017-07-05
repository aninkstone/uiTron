#ifndef _H8_3069_H_
#define _H8_3069_H_
/*
 * h8_3069.h : Register definition for H8/3069
 *
 * Copyright (C) 2003 Naonori Tazawa (int@kt.rim.or.jp)
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

#define P1DDR           (*(volatile unsigned char  *)0xfee000)
#define P2DDR           (*(volatile unsigned char  *)0xfee001)
#define P3DDR           (*(volatile unsigned char  *)0xfee002)
#define P4DDR           (*(volatile unsigned char  *)0xfee003)
#define P5DDR           (*(volatile unsigned char  *)0xfee004)
#define P6DDR           (*(volatile unsigned char  *)0xfee005)
#define P8DDR           (*(volatile unsigned char  *)0xfee007)
#define P9DDR           (*(volatile unsigned char  *)0xfee008)
#define PADDR           (*(volatile unsigned char  *)0xfee009)
#define PBDDR           (*(volatile unsigned char  *)0xfee00a)
#define MDCR            (*(volatile unsigned char  *)0xfee011)
#define SYSCR           (*(volatile unsigned char  *)0xfee012)
#define BRCR            (*(volatile unsigned char  *)0xfee013)
#define ISCR            (*(volatile unsigned char  *)0xfee014)
#define IER             (*(volatile unsigned char  *)0xfee015)
#define ISR             (*(volatile unsigned char  *)0xfee016)
#define IPRA            (*(volatile unsigned char  *)0xfee018)
#define IPRB            (*(volatile unsigned char  *)0xfee019)
#define DASTCR          (*(volatile unsigned char  *)0xfee01A)
#define DIVCR           (*(volatile unsigned char  *)0xfee01B)
#define MSTCRH          (*(volatile unsigned char  *)0xfee01C)
#define MSTCRL          (*(volatile unsigned char  *)0xfee01D)
#define CSCR            (*(volatile unsigned char  *)0xfee01f)
#define ABWCR           (*(volatile unsigned char  *)0xfee020)
#define ASTCR           (*(volatile unsigned char  *)0xfee021)
#define WCRH            (*(volatile unsigned char  *)0xfee022)
#define WCRL            (*(volatile unsigned char  *)0xfee023)
#define BCR             (*(volatile unsigned char  *)0xfee024)
#define DRCRA           (*(volatile unsigned char  *)0xfee026)
#define DRCRB           (*(volatile unsigned char  *)0xfee027)
#define RTMCSR          (*(volatile unsigned char  *)0xfee028)
#define RTCNT           (*(volatile unsigned char  *)0xfee029)
#define RTCOR           (*(volatile unsigned char  *)0xfee02a)
#define FLMCR           (*(volatile unsigned char  *)0xfee030)
#define EBR             (*(volatile unsigned char  *)0xfee032)
#define P2PCR           (*(volatile unsigned char  *)0xfee03c)
#define P4PCR           (*(volatile unsigned char  *)0xfee03e)
#define P5PCR           (*(volatile unsigned char  *)0xfee03f)
#define RAMCR           (*(volatile unsigned char  *)0xfee077)
#define FLMSR           (*(volatile unsigned char  *)0xfee07d)
#define MAR0AR          (*(volatile unsigned char  *)0xffff20)
#define MAR0AE          (*(volatile unsigned char  *)0xffff21)
#define MAR0AH          (*(volatile unsigned char  *)0xffff22)
#define MAR0AL          (*(volatile unsigned char  *)0xffff23)
#define ETCR0AH         (*(volatile unsigned char  *)0xffff24)
#define ETCR0AL         (*(volatile unsigned char  *)0xffff25)
#define IOAR0A          (*(volatile unsigned char  *)0xffff26)
#define DTCR0A          (*(volatile unsigned char  *)0xffff27)
#define MAR0BR          (*(volatile unsigned char  *)0xffff28)
#define MAR0BE          (*(volatile unsigned char  *)0xffff29)
#define MAR0BH          (*(volatile unsigned char  *)0xffff2a)
#define MAR0BL          (*(volatile unsigned char  *)0xffff2b)
#define ETCR0BH         (*(volatile unsigned char  *)0xffff2c)
#define ETCR0BL         (*(volatile unsigned char  *)0xffff2d)
#define IOAR0B          (*(volatile unsigned char  *)0xffff2e)
#define DTCR0B          (*(volatile unsigned char  *)0xffff2f)
#define MAR1AR          (*(volatile unsigned char  *)0xffff30)
#define MAR1AE          (*(volatile unsigned char  *)0xffff31)
#define MAR1AH          (*(volatile unsigned char  *)0xffff32)
#define MAR1AL          (*(volatile unsigned char  *)0xffff33)
#define ETCR1AH         (*(volatile unsigned char  *)0xffff34)
#define ETCR1AL         (*(volatile unsigned char  *)0xffff35)
#define IOAR1A          (*(volatile unsigned char  *)0xffff36)
#define DTCR1A          (*(volatile unsigned char  *)0xffff37)
#define MAR1BR          (*(volatile unsigned char  *)0xffff38)
#define MAR1BE          (*(volatile unsigned char  *)0xffff39)
#define MAR1BH          (*(volatile unsigned char  *)0xffff3a)
#define MAR1BL          (*(volatile unsigned char  *)0xffff3b)
#define ETCR1BH         (*(volatile unsigned char  *)0xffff3c)
#define ETCR1BL         (*(volatile unsigned char  *)0xffff3d)
#define IOAR1B          (*(volatile unsigned char  *)0xffff3e)
#define DTCR1B          (*(volatile unsigned char  *)0xffff3f)
#define TSTR            (*(volatile unsigned char  *)0xffff60)
#define TSNC            (*(volatile unsigned char  *)0xffff61)
#define TMDR            (*(volatile unsigned char  *)0xffff62)
#define TOLR            (*(volatile unsigned char  *)0xffff63)
#define TISRA           (*(volatile unsigned char  *)0xffff64)
#define TISRB           (*(volatile unsigned char  *)0xffff65)
#define TISRC           (*(volatile unsigned char  *)0xffff66)
#define TCR0            (*(volatile unsigned char  *)0xffff68)
#define TIOR0           (*(volatile unsigned char  *)0xffff69)
#define TCNT0           (*(volatile unsigned short *)0xffff6a)
#define GRA0            (*(volatile unsigned short *)0xffff6c)
#define GRB0            (*(volatile unsigned short *)0xffff6e)
#define TCR1            (*(volatile unsigned char  *)0xffff70)
#define TIOR1           (*(volatile unsigned char  *)0xffff71)
#define TCNT1           (*(volatile unsigned short *)0xffff72)
#define GRA1            (*(volatile unsigned short *)0xffff74)
#define GRB1            (*(volatile unsigned short *)0xffff76)
#define TCR2            (*(volatile unsigned char  *)0xffff78)
#define TIOR2           (*(volatile unsigned char  *)0xffff79)
#define TCNT2           (*(volatile unsigned short *)0xffff7a)
#define GRA2            (*(volatile unsigned short *)0xffff7c)
#define GRB2            (*(volatile unsigned short *)0xffff7e)
#define TCR0_8          (*(volatile unsigned char  *)0xffff80)
#define TCR1_8          (*(volatile unsigned char  *)0xffff81)
#define TCSR0_8         (*(volatile unsigned char  *)0xffff82)
#define TCSR1_8         (*(volatile unsigned char  *)0xffff83)
#define TCORA0_8        (*(volatile unsigned char  *)0xffff84)
#define TCORA1_8        (*(volatile unsigned char  *)0xffff85)
#define TCORB0_8        (*(volatile unsigned char  *)0xffff86)
#define TCORB1_8        (*(volatile unsigned char  *)0xffff87)
#define TCNT0_8         (*(volatile unsigned char  *)0xffff88)
#define TCNT1_8         (*(volatile unsigned char  *)0xffff89)
#define TCSR            (*(volatile unsigned char  *)0xffff8c)
#define TCNT            (*(volatile unsigned char  *)0xffff8d)
#define RSTCSR          (*(volatile unsigned char  *)0xffff8f)
#define TCR2_8          (*(volatile unsigned char  *)0xffff90)
#define TCR3_8          (*(volatile unsigned char  *)0xffff91)
#define TCSR2_8         (*(volatile unsigned char  *)0xffff92)
#define TCSR3_8         (*(volatile unsigned char  *)0xffff93)
#define TCORA2_8        (*(volatile unsigned char  *)0xffff94)
#define TCORA3_8        (*(volatile unsigned char  *)0xffff95)
#define TCORB2_8        (*(volatile unsigned char  *)0xffff96)
#define TCORB3_8        (*(volatile unsigned char  *)0xffff97)
#define TCNT2_8         (*(volatile unsigned char  *)0xffff98)
#define TCNT3_8         (*(volatile unsigned char  *)0xffff99)
#define DADR0           (*(volatile unsigned char  *)0xffff9c)
#define DADR1           (*(volatile unsigned char  *)0xffff9d)
#define DACR            (*(volatile unsigned char  *)0xffff9e)
#define TPMR            (*(volatile unsigned char  *)0xffffa0)
#define TPCR            (*(volatile unsigned char  *)0xffffa1)
#define NDERB           (*(volatile unsigned char  *)0xffffa2)
#define NDERA           (*(volatile unsigned char  *)0xffffa3)
#define NDRB_1          (*(volatile unsigned char  *)0xffffa4)
#define NDRA_1          (*(volatile unsigned char  *)0xffffa5)
#define NDRB_2          (*(volatile unsigned char  *)0xffffa6)
#define NDRA_2          (*(volatile unsigned char  *)0xffffa7)
#define SMR0            (*(volatile unsigned char  *)0xffffb0)
#define BRR0            (*(volatile unsigned char  *)0xffffb1)
#define SCR0            (*(volatile unsigned char  *)0xffffb2)
#define TDR0            (*(volatile unsigned char  *)0xffffb3)
#define SSR0            (*(volatile unsigned char  *)0xffffb4)
#define RDR0            (*(volatile unsigned char  *)0xffffb5)
#define SCMR0           (*(volatile unsigned char  *)0xffffb6)
#define SMR1            (*(volatile unsigned char  *)0xffffb8)
#define BRR1            (*(volatile unsigned char  *)0xffffb9)
#define SCR1            (*(volatile unsigned char  *)0xffffba)
#define TDR1            (*(volatile unsigned char  *)0xffffbb)
#define SSR1            (*(volatile unsigned char  *)0xffffbc)
#define RDR1            (*(volatile unsigned char  *)0xffffbd)
#define SCMR1           (*(volatile unsigned char  *)0xffffbe)
#define SMR2            (*(volatile unsigned char  *)0xffffc0)
#define BRR2            (*(volatile unsigned char  *)0xffffc1)
#define SCR2            (*(volatile unsigned char  *)0xffffc2)
#define TDR2            (*(volatile unsigned char  *)0xffffc3)
#define SSR2            (*(volatile unsigned char  *)0xffffc4)
#define RDR2            (*(volatile unsigned char  *)0xffffc5)
#define SCMR2           (*(volatile unsigned char  *)0xffffc6)
#define P1DR            (*(volatile unsigned char  *)0xffffd0)
#define P2DR            (*(volatile unsigned char  *)0xffffd1)
#define P3DR            (*(volatile unsigned char  *)0xffffd2)
#define P4DR            (*(volatile unsigned char  *)0xffffd3)
#define P5DR            (*(volatile unsigned char  *)0xffffd4)
#define P6DR            (*(volatile unsigned char  *)0xffffd5)
#define P7DR            (*(volatile unsigned char  *)0xffffd6)
#define P8DR            (*(volatile unsigned char  *)0xffffd7)
#define P9DR            (*(volatile unsigned char  *)0xffffd8)
#define PADR            (*(volatile unsigned char  *)0xffffd9)
#define PBDR            (*(volatile unsigned char  *)0xffffda)
#define ADDRA           (*(volatile unsigned short *)0xffffe0)
#define ADDRB           (*(volatile unsigned short *)0xffffe2)
#define ADDRC           (*(volatile unsigned short *)0xffffe4)
#define ADDRD           (*(volatile unsigned short *)0xffffe6)
#define ADDRAH          (*(volatile unsigned char  *)0xffffe0)
#define ADDRAL          (*(volatile unsigned char  *)0xffffe1)
#define ADDRBH          (*(volatile unsigned char  *)0xffffe2)
#define ADDRBL          (*(volatile unsigned char  *)0xffffe3)
#define ADDRCH          (*(volatile unsigned char  *)0xffffe4)
#define ADDRCL          (*(volatile unsigned char  *)0xffffe5)
#define ADDRDH          (*(volatile unsigned char  *)0xffffe6)
#define ADDRDL          (*(volatile unsigned char  *)0xffffe7)
#define ADCSR           (*(volatile unsigned char  *)0xffffe8)
#define ADCR            (*(volatile unsigned char  *)0xffffe9)

#define TCR01_8         (*(volatile unsigned short *)0xffff80)
#define TCSR01_8        (*(volatile unsigned short *)0xffff82)
#define TCORA01_8       (*(volatile unsigned short *)0xffff84)
#define TCORB01_8       (*(volatile unsigned short *)0xffff86)
#define TCNT01_8        (*(volatile unsigned short *)0xffff88)

#define TCR23_8         (*(volatile unsigned short *)0xffff90)
#define TCSR23_8        (*(volatile unsigned short *)0xffff92)
#define TCORA23_8       (*(volatile unsigned short *)0xffff94)
#define TCORB23_8       (*(volatile unsigned short *)0xffff96)
#define TCNT23_8        (*(volatile unsigned short *)0xffff98)

#define INT_start     0
#define INT_Reserved  1
#define INT_NMI       7
#define INT_TRAP0     8
#define INT_TRAP1     9
#define INT_TRAP2     10
#define INT_TRAP3     11
#define INT_IRQ0      12
#define INT_IRQ1      13
#define INT_IRQ2      14
#define INT_IRQ3      15
#define INT_IRQ4      16
#define INT_IRQ5      17
#define INT_WOVI      20
#define INT_CMI       21
#define INT_ADI       23
#define INT_IMIA0     24
#define INT_IMIB0     25
#define INT_OVI0      26
#define INT_IMIA1     28
#define INT_IMIB1     29
#define INT_OVI1      30
#define INT_IMIA2     32
#define INT_IMIB2     33
#define INT_OVI2      34
#define INT_CMIA0     36
#define INT_CMIB0     37
#define INT_CMIA1B1   38
#define INT_TOVI01    39
#define INT_CMIA2     40
#define INT_CMIB2     41
#define INT_CMIA3B3   42
#define INT_TOVI23    43
#define INT_DEND0A    44
#define INT_DEND0B    45
#define INT_DEND1A    46
#define INT_DEND1B    47
#define INT_ERI0      52
#define INT_RXI0      53
#define INT_TXI0      54
#define INT_TEI0      55
#define INT_ERI1      56
#define INT_RXI1      57
#define INT_TXI1      58
#define INT_TEI1      59
#define INT_ERI2      60
#define INT_RXI2      61
#define INT_TXI2      62
#define INT_TEI2      63

#define H8_INRAM_AD   0xffbf20
#define H8_INRAM_SZ   16384

#endif /* _H8_3069_H_ */
