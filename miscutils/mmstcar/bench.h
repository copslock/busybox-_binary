/****************************************************************
 * $ID: bench.h          Tue, Nov 11 10:49:17 2003 CST  mhfan $ *
 *								*
 * Description:							*
 *								*
 * Author:      范美辉(MeiHui FAN) <mhfan@ustc.edu>		*
 *								*
 * Maintainer:  M.H.Fan  <mhfan@ustc.edu>			*
 *              Laboratory of Structural Biology		*
 *              School of Life Science				*
 *              Univ. of Sci.& Tech. of China (USTC)		*
 *              People's Republic of China (PRC)		*
 *								*
 * CopyRight (c)  2003~2009  M.H.Fan				*
 *   All rights reserved.					*
 ****************************************************************/
#ifndef BENCH_H
#define BENCH_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define	EPSILON			    0.000001F

#ifndef uint64_t
#define int64_t			    long long int
#define uint64_t		    unsigned long long int
#endif//uint64_t

/*
 * 在 Intel Pentium 以上级别（及其兼容型）的 CPU 中，有一个称为时间戳
 * (Time Stamp) 的部件，它以 64 位无符号整型数的格式，记录了自 CPU 上电以来
 * 所经过的时钟周期数。
 *
 * X86 指令集提供了一条机器指令 RDTSC (Read Time Stamp Counter) 来读取这个
 * 时间戳的数字，并将其保存在 EDX:EAX 寄存器对中。由于 EDX:EAX 寄存器对恰好是
 * Win32/Linux 平台下 C++ 语言保存函数返回值的寄存器，所以我们可以把这条指令
 * 看成是一个普通的函数调用。
 */

inline uint64_t rdtsc()
{
    uint64_t dst;
    asm volatile ("RDTSC\n" : "=A" (dst));	// compatible with ANSI C.
    // asm volatile ("RDTSC");
    // asm ("_emit 0x0F\r\t"	"_emit 0x31");	// for cpp
    // asm ("_emit 0x0F; _emit 0x31");
    return dst;
}

float get_cpu_freq()
{
    uint64_t freq = rdtsc(); sleep(1); return freq = rdtsc() - freq;
}

float getCpuFreq()
{
    size_t n=64;
    char* lineptr = (char*)malloc(n);
    FILE* fp = fopen("/proc/cpuinfo", "r");
    do { getdelim(&lineptr, &n, '\n', fp);
    } while (0 != memcmp (lineptr, "cpu MHz", 7));
    n = 0;	while (lineptr[++n] != ':') ;
    fclose (fp);    return atof(lineptr+n+1);
}

float cpu_freq = 0.0F;

double cyc2sec (uint64_t cyc)
{
    if (cpu_freq < EPSILON) cpu_freq = get_cpu_freq ();
    //int64_t call_interval = rdtsc() - rdtsc();
    return (cyc/* - call_interval*/) / (double)cpu_freq;
}

#ifdef	DEBUG
void print (void)
{
    fprintf (stdout, "CPU clock cycle count: %Ld\n", rdtsc ());
    return;
}
#endif//DEBUG

#endif//BENCH_H
// vim:sts=4:ts=8:
