/*
 * tprintf.c : Tiny printf functions
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
#include <stdarg.h>
#include "tprintf.h"
#ifdef _ROM_
# include "sci.h"
#endif

typedef unsigned int uint;
typedef void (*FNP)(char, VP);

void put_ch(char c, VP dmy)
{
#ifdef _ROM_
    put_sci(c);
#else
    __asm__ volatile (
        "push.l	er1\n"
        "mov.l	er0, er1\n"
        "sub.l	er0, er0\n"
        "trapa	#1\n"
        "pop.l	er1\n"
        );
#endif
}

static void put_mem(char c, char **p)
{
    *(*p)++ = c;
}

static int strlen(char *p)
{
    int l = 0;
    
    while (*p++)
        l++;
    return l;
}

static void vsubs(FNP fnp, char *p, VP par)
{
    while(*p)
        (fnp)(*p++, par);
}

static void vsubp(FNP fnp, int c, VP par, int len)
{
    while(len-- > 0)
        (fnp)(c, par);
}

static int t_vprintf(FNP fnp, VP par, const char *f, va_list v)
{
    int c, w;
    int mod = 0;
    char *p;
    char buf[12];
    int sgn;
    int zero;
    int width;
    int left;
    int cnt = 0;

    while (c = *f++) {
        if (mod == 0) {
            if (c == '%') {
                sgn = 0;
                zero = 0;
                width = 0;
                left = 0;
                mod++;
                continue;
            }
            (fnp)(c, par);
        } else {
            switch (c) {
            case 'c' :
                w = va_arg(v, int);
                (fnp)(w, par);
                mod--;
                cnt++;
                break;

            case 's' :
                p = (char *)va_arg(v, int);
                mod--;
                zero = 0;
                goto FIELD;
                
            case 'd' :
                sgn = 1;
            case 'u' :
            {
                uint d, d1;
#ifdef BIT16
                uint dig = 10000;
#else
                uint dig = 1000000000;
#endif
                p = buf;
                d = va_arg(v, int);
                if (sgn) {
                    if ((int)d < 0) {
                        d = ~d + 1;
                    } else {
                        sgn = 0;
                    }
                }
                while(dig) {
                    d1 = d / dig;
                    d -= d1 * dig;
                    dig /= 10;
                    *p++ = '0' + d1;
                }
                *p = 0;
                for (p = buf; *p == '0'; p++)
                    ;
                if (!*p)
                    p--;
                mod--;
                goto FIELD;
            }

            case 'x' :
            {
                uint d, d1;
#ifdef BIT16
                uint dig = 0x1000;
#else
                uint dig = 0x10000000;
#endif
                p = buf;
                d = va_arg(v, int);
                while(dig) {
                    d1 = d / dig;
                    d -= d1 * dig;
                    dig >>= 4;
                    *p++ = (d1 < 10)? '0' + d1 : 'a' + d1 - 10;
                }
                *p = 0;
                for (p=buf; *p == '0'; p++)
                    ;
                if (!*p)
                    p--;
                mod--;
                goto FIELD;
            }
                
            case '-' :
                left = 1;
                break;
                
            case '0' :
                if (width == 0) {
                    zero = 1;
                    break;
                }
                
            default :
                if ('0' <= c && c <= '9')
                    width = width * 10 + (c - '0');
                break;
                
            FIELD:
                if (sgn)
                    (fnp)('-', par);
                if (width == 0) {
                    vsubs(fnp, p, par);
                } else {
                    w = width - strlen(p);
                    if (left) {
                        vsubs(fnp, p, par);
                        vsubp(fnp, ' ', par, w);
                    } else {
                        vsubp(fnp, (zero)?'0':' ', par, w);
                        vsubs(fnp, p, par);
                    }
                }
                cnt++;
            }
        }
    }
    return cnt;
}

int t_printf(const char *f, ...)
{
    va_list v;
    int r;

    va_start(v, f);
    r = t_vprintf(put_ch, 0, f, v);
    va_end(v);
    return r;
}

int t_fprintf(int ch, const char *f, ...)
{
    va_list v;
    int r;

    va_start(v, f);
    r = t_vprintf((FNP)put_ch, (VP)ch, f, v);
    va_end(v);
    return r;
}

int t_sprintf(char *s, const char *f, ...)
{
    va_list v;
    int r;

    va_start(v, f);
    r = t_vprintf((FNP)put_mem, &s, f, v);
    *s = '\0';
    va_end(v);
    return r;
}

void _kernel_panic(char *file, int line)
{
    vdis_psw();
    t_printf("KERNEL PANIC : %s:%d\n", file, line);
    _KERNEL_TRAP_GDB();
}

#ifdef _KERNEL_ASSERT
void _kernel_assert(char *f, ...)
{
    va_list v;
    int r;

    vdis_psw();
    t_printf("\nKERNEL ASSERT : ");
    va_start(v, f);
    r = t_vprintf(put_ch, 0, f, v);
    va_end(v);
    _KERNEL_TRAP_GDB();
}
#endif

/* end */
