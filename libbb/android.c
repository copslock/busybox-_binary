//#!/usr/bin/tcc -run
/****************************************************************
 * $ID: android.c      ËÄ, 25 11ÔÂ 2010 11:29:13 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ·¶ÃÀ»Ô(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyLeft (c)  2010  M.H.Fan                                  *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/

#include "libbb.h"

// declared in stdlib.h
int clearenv(void)
{
    environ = NULL;
    return 0;
}

struct mntent *getmntent_r(FILE *fp, struct mntent *mnt, char *buf, int buflen)
{
	char *tokp = NULL, *s;

	do {
		if (!fgets(buf, buflen, fp))
			return NULL;
		tokp = 0;
		s = strtok_r(buf, " \t\n", &tokp);
	} while (!s || *s == '#');

	mnt->mnt_fsname = s;
	mnt->mnt_freq = mnt->mnt_passno = 0;
	if (!(mnt->mnt_dir = strtok_r(NULL, " \t\n", &tokp)))
		return NULL;
	if (!(mnt->mnt_type = strtok_r(NULL, " \t\n", &tokp)))
		return NULL;
	if (!(mnt->mnt_opts = strtok_r(NULL, " \t\n", &tokp)))
		mnt->mnt_opts = (char*)"";
	else if ((s = strtok_r(NULL, " \t\n", &tokp)))
	{
		mnt->mnt_freq = atoi(s);
		if ((s = strtok_r(NULL, " \t\n", &tokp)))
			mnt->mnt_passno = atoi(s);
	}

	return mnt;
}

#ifndef BIONIC_ICS
// no /etc/shells anyway
char *getusershell(void) { return NULL; }
void setusershell(void) {}
void endusershell(void) {}

// override definition in bionic/stubs.c
struct mntent *getmntent(FILE *fp)
{
    static struct mntent mnt;
    static char buf[256];
    return getmntent_r(fp, &mnt, buf, 256);
}
#endif

// not used anyway
int addmntent(FILE *fp, const struct mntent *mnt)
{
    (void)fp; (void)mnt;
    errno = ENOENT;
    return 1;
}

// declared in grp.h, but not necessary
int setpwent(void) { return 0; }
void setgrent(void) {}

void *mempcpy(void *dest, const void *src, size_t n)
{
    memcpy(dest, src, n);
    return (unsigned char*)dest + n;
}

int fdatasync(int fd)
{
    return syscall(__NR_fdatasync, fd);
}

int sethostname(const char* name, size_t len)
{
    return syscall(__NR_sethostname, name, len);
}

int cfsetspeed(struct termios *termios_p, speed_t speed)
{
    speed_t news;
    news = cfsetispeed(termios_p, speed);
    if (news != speed) return -1;
    news = cfsetospeed(termios_p, speed);
    if (news != speed) return -1;
    return 0;
}

int tcdrain(int fd)
{
    return tcflush(fd, TCOFLUSH);
}

int pivot_root(const char* new_root, const char* put_old);
int pivot_root(const char* new_root, const char* put_old)
{
    return syscall(__NR_pivot_root, new_root, put_old);
}

// XXX: copy from uClibc

//#define __FORCE_GLIBC
//#include <features.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>

struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr)
{
	/* asc is "X:XX:XX:x:xx:xX" */
	int cnt;

	for (cnt = 0; cnt < 6; ++cnt) {
		unsigned char number;
		char ch;

		/* | 0x20 is cheap tolower(), valid for letters/numbers only */
		ch = (*asc++) | 0x20;
		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
			return NULL;
		number = !(ch > '9') ? (ch - '0') : (ch - 'a' + 10);

		ch = *asc++;
		if ((cnt != 5 && ch != ':') /* not last group */
		/* What standard says ASCII ether address representation
		 * may also finish with whitespace, not only NUL?
		 * We can get rid of isspace() otherwise */
		 || (cnt == 5 && ch != '\0' /*&& !isspace(ch)*/)
		) {
			ch |= 0x20; /* cheap tolower() */
			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
				return NULL;
			number = (number << 4) + (!(ch > '9') ? (ch - '0') : (ch - 'a' + 10));

			if (cnt != 5) {
				ch = *asc++;
				if (ch != ':')
					return NULL;
			}
		}

		/* Store result.  */
		addr->ether_addr_octet[cnt] = number;
	}
	/* Looks like we allow garbage after last group?
	 * "1:2:3:4:5:66anything_at_all"? */

	return addr;
}
//libc_hidden_def(ether_aton_r)

struct ether_addr *ether_aton(const char *asc)
{
	static struct ether_addr result;

	return ether_aton_r(asc, &result);
}

char *ether_ntoa_r(const struct ether_addr *addr, char *buf)
{
	sprintf(buf, "%x:%x:%x:%x:%x:%x",
			addr->ether_addr_octet[0], addr->ether_addr_octet[1],
			addr->ether_addr_octet[2], addr->ether_addr_octet[3],
			addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
	return buf;
}
//libc_hidden_def(ether_ntoa_r)

char *ether_ntoa(const struct ether_addr *addr)
{
	static char asc[18];

	return ether_ntoa_r(addr, asc);
}

int sigisemptyset(sigset_t *set)
{
    int  sig;
    for (sig = 0; sig < NSIG; ++sig) if (sigismember(set, sig)) return 0;
    return 1;
}

// vim:sts=4:ts=8:
