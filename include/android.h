/****************************************************************
 * $ID: android.h      ËÄ, 25 11ÔÂ 2010 11:31:17 +0800  mhfan $ *
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
#ifndef ANDROID_H
#define ANDROID_H

#include <stdio.h>
#include <termios.h>
#include <sys/syscall.h>

#define msgctl(msgid, cmd, buf) \
	syscall(__NR_msgctl, msgid, cmd, buf)

#define msgget(key, msgflg) \
	syscall(__NR_msgget, key, msgflg)

#define semctl(semid, cmd, buf, ...) \
	syscall(__NR_semctl, semid, cmd, buf, __VA_ARGS__)

#define semget(key, nsems, semflg) \
	syscall(__NR_semget, key, nsems, semflg)

#define semop(semid, sops, nsops) \
	syscall(__NR_semop, semid, sops, nsops)

#define shmctl(shmid, cmd, buf) \
	syscall(__NR_shmctl, shmid, cmd, buf)

#define shmget(key, size, shmflg) \
	syscall(__NR_shmget, key, size, shmflg)

#define shmat(shmid, shmaddr, shmflg) \
	(void*)syscall(__NR_shmat, shmid, shmaddr, shmflg)

#define shmdt(shmaddr) \
	syscall(__NR_shmdt, shmaddr)

#define adjtimex(buf) \
	syscall(__NR_adjtimex, buf)

#define sigtimedwait(set, info, timeout) \
	syscall(__NR_rt_sigtimedwait, set, info, timeout)

#define sigwaitinfo(set, info) sigtimedwait(set, info, NULL)

#define readahead(fd, offset, count) \
	syscall(__NR_readahead, fd, offset, count)

#if 0
#define readlinkat(dirfd, pathname, buf, bufsize) \
	syscall(__NR_readlinkat, dirfd, pathname, buf, bufsize)

#define signalfd(fd, mask, flags) \
	syscall(__NR_signalfd, fd, mask, flags)

#define stime(t) \
	syscall(__NR_stime, t)

#define swapon(path, swapflags) \
	syscall(__NR_swapon, path, swapflags)

#define swapoff(path) \
	syscall(__NR_swapoff, path)

#define sysinfo(info) \
	syscall(__NR_sysinfo, info)

#define getsid(pid) \
	syscall(__NR_getsid, pid)
#endif

// should be in sys/stat.h, bits/stat.h
#define S_TYPEISMQ(buf)  ((buf)->st_mode - (buf)->st_mode)
#define S_TYPEISSEM(buf) ((buf)->st_mode - (buf)->st_mode)
#define S_TYPEISSHM(buf) ((buf)->st_mode - (buf)->st_mode)

#define setmntent fopen
#define endmntent fclose

#ifndef RUN_LVL
// should be in bits/utmp.h
#define RUN_LVL 1
#endif

void endutent(void);	// defined in bionic/utmp.c
char *mkdtemp(char *);	// defined in bionic/mktemp.c

struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr);

int stime(time_t *);
int swapon(const char *, int);
int swapoff(const char *);

struct mntent;
struct __sFILE;

int addmntent(struct __sFILE *, const struct mntent *);
struct mntent *getmntent_r(struct __sFILE *fp, struct mntent *mnt,
	char *buf, int buflen);

void *mempcpy(void *dest, const void *src, size_t n);

int cfsetspeed(struct termios *termios_p, speed_t speed);
int tcdrain(int fd);

pid_t getsid(pid_t pid);
int sigisemptyset(sigset_t *set);
int sethostname(const char* name, size_t len);
char *ether_ntoa_r(const struct ether_addr *addr, char *buf);

// bionic's vfork is rather broken; for now a terrible bandaid:
//#define vfork fork

#endif//ANDROID_H
// vim:sts=4:ts=8:
