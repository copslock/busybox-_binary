/****************************************************************
 * $ID: property.h     Thu, 31 Jul 2008 09:28:21 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyRight (c)  2009  M.H.Fan                                 *
 *   All rights reserved.                                       *
 ****************************************************************/
#ifndef PROPERTY_H
#define PROPERTY_H

#define DEBUG
#define SANITY_WRAP

#include "errmsg.h"

#include <time.h>
#include <poll.h>
#include <signal.h>
#include <termios.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define fdprintf(STDOUT_FILENO, ...) fprintf(stdout, __VA_ARGS__)
//#define fdprintf(STDERR_FILENO, ...) fprintf(stderr, __VA_ARGS__)

#if 0
#define full_write(...)		write(__VA_ARGS__)
#define safe_read(...)		read (__VA_ARGS__)
#define safe_poll(...)		poll (__VA_ARGS__)
#define open_or_warn(...)	open(__VA_ARGS__)
#else// XXX:
#define full_write(...)		sanity_iowrap(write, __VA_ARGS__)
#define safe_read(...)		sanity_iowrap(read,  __VA_ARGS__)
#define safe_poll(...)		sanity_fswrap(poll,  __VA_ARGS__)
#define open_or_warn(...)	sanity_fswrap(open,  __VA_ARGS__)
#endif

typedef int smallint;

#define COMMON_BUFSIZE 4096
static char bb_common_bufsiz1[COMMON_BUFSIZE];

static void xopen_xwrite_close(const char* fp, const char* str)
{
    int fd = open_or_warn(fp, O_WRONLY);
    full_write(fd, str, strlen(str));	close(fd);
}

static speed_t tty_value_to_baud(unsigned val)
{
    switch (val) {
    case 115200: return B115200;
    case  19200: return B19200;
    case   4800: return B4800;
    default: ;	// TODO:
    }	dtrace; return 0;
}

static void bb_signals(unsigned sigs, void (*sigh)(int))
{
    unsigned sig =  0x00, sigm;
    while ((sigm = (0x01 << ++sig)) < sigs)
	if (sigm & sigs) signal(sig, sigh);
}

static uint32_t getopt32(char* *argv, const char *applet_opts,
	const char* *first, ...)
{   // FIXME:
    unsigned opts = 0x00;
    if ((*first = argv[1])) opts = 0x01;
    return opts;
}

#endif//PROPERTY_H
// vim:sts=4:ts=8:
