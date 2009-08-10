//#!/usr/bin/tcc -run
/****************************************************************
 * $ID: fwutil.c       Ò», 13  9ÔÂ 2010 15:38:02 +0800  mhfan $ *
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

//kbuild:lib-y :=
//kbuild:lib-$(CONFIG_FWUTIL) += fwutil.o
//kbuild:EXTRA_CFLAGS += -Wno-multichar #-Wno-unused -Wstrict-aliasing=2
//kbuild:
//config:config FWUTIL
//config:       bool "fwutil"
//config:       default n
//config:       help
//config:         Firmware utiltility for creating, appending, dumping
//config:         and flashing usage.
//config:
//applet:IF_FWUTIL(APPLET(fwutil, _BB_DIR_SBIN, _BB_SUID_DROP))
//applet:
//usage:#define fwutil_trivial_usage
//usage:        "-[idn] FIRMWARE <SUB-OPTIONS>"
//usage:#define fwutil_full_usage "\n\n"
//usage:        "Options:"
//usage:     "\n        -i FIRMWARE [-c]"
//usage:     "\n                Print firmware informations"
//usage:     "\n        -d FIRMWARE -p COMPONENT[:#.#] ..."
//usage:     "\n                Dump/flash firmware component(s)"
//usage:     "\n        -n FIRMWARE -v M.N.R -p COMPONENT[:#.#] ..."
//usage:     "\n                Create new firmware"
//usage-     "\n        -a FIRMWARE -p COMPONENT[:#.#] ..."
//usage-     "\n                Append new component(s)"
//usage:

// XXX: CONFIG_USE_BB_CRYPT
#include <rpc/des_crypt.h>

#include <linux/fs.h>	// XXX: for BLKGETSIZE64

#include "libbb.h"

#include "fwutil.h"

int fwutil_main(int argc, char* argv[]) MAIN_EXTERNALLY_VISIBLE;
int fwutil_main(int argc, char* argv[])
{
    //char *path = NULL, *sign = NULL;
    llist_t *compath = NULL;
    fw_hdr *fwh, fwhdr;
    char *vers = NULL;
    uint32_t opts;

    enum {
	OPT_DUMP	= 0,
	OPT_PRINT	= 1,
	OPT_CREATE	= 2,
	OPT_APPEND	= 3,
	OPT_PART	= 4,
	OPT_VERSION	= 5,
	OPT_SIGNATURE	= 6,
	OPT_CHECK	= 7,
	OPT_HELP	= 8
#define OPT_MASK(a)	(0x01 << a)
    };

    memset(fwh = &fwhdr, 0, sizeof(*fwh));

    opt_complementary =
	    "p::d--nias:n--diac:i--ndas:a--ndis:n?vp:d?p:i:n:d:a:h-";
    opts = getopt32(argv, "d:i:n:a:p:v:s:ch", &fwh->path, &fwh->path,
	    &fwh->path, &fwh->path, &compath, &vers, &fwh->sign);
    //argc -= optind;	argv += optind;

    //fwh->path  = strdup(path);
    fwh->stamp = 0x00;

    // FIXME: die_jmp;
    die_sleep = -1;

    if (opts & OPT_MASK(OPT_PRINT)) {
	if ((argc = fw_open(fwh))) return argc;
	fw_print(fwh);

	if (opts & OPT_MASK(OPT_CHECK)) fw_check(fwh);

	fw_close(fwh);
	return 0;
    } else

    if (opts & OPT_MASK(OPT_CREATE)) {
	fwh->stamp = 0x01;

	if (vers) {
	    fwh->vers.major = strtol(  vers, &vers, 10);
	    fwh->vers.minor = strtol(++vers, &vers, 10);
	    fwh->vers.rev   = strtol(++vers, &vers, 10);
	} else fwh->vers._  = 0;

	fwh->vendor = /*strdup*/(char*)(FIRMWARE_VENDOR);
	//if (sign) fwh->sign = /*strdup*/(sign);
    } else

    if (opts & OPT_MASK(OPT_APPEND)) {
	fwh->stamp = 0x02;
    } else

    if (opts & OPT_MASK(OPT_DUMP)) {
    } else ;

    if (compath) {
	fw_comp *fwc, fwcomp;

	fwh->fd = -1;
	if ((argc = fw_open(fwh))) return argc;
	memset(fwc = &fwcomp, 0, sizeof(*fwc));

	if (opts & OPT_MASK(OPT_APPEND)) {  fw_comp* tmp;
	    for (tmp = fwh->first; tmp && tmp->next; tmp = tmp->next) ;
	    if  (tmp) fwc->info.part = tmp->info.part;
	}

	do { fwc->path = llist_pop(&compath);

	    if ((vers = strrchr(fwc->path, ':'))) {	*vers = '\0';
		fwc->info.part    = strtol(++vers, &vers, 10);
		fwc->info.deflate = strtol(++vers, &vers, 10);
	    } else {
		++fwc->info.part;	fwc->info.deflate = 0;	// XXX:
	    }

	    if (opts & OPT_MASK(OPT_DUMP)) {	fw_comp* tmp;
		if ((tmp = fw_find(fwh, fwc->info))) {
		     tmp->path = fwc->path;
		    fw_output(fwh, tmp);
		}
	    } else
	    //if (opts & (OPT_MASK(OPT_CREATE) | OPT_MASK(OPT_APPEND)))
		if ((argc = fw_append(fwh, fwc))) return argc;
	} while (compath);

	fw_close(fwh);
    }

    return 0;
}

off_t FAST_FUNC fdlength(int fd)
{ // XXX:
    struct stat st;
    off_t len;

    if (fstat(fd, &st)) {
	st.st_size = lseek(fd, 0, SEEK_CUR);
	if ((len = lseek(fd, 0, SEEK_END)) < 0) {
	    const char* msg = "lseek failed";
	    if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		    msg, strerror(errno)); else
	    bb_simple_perror_msg(msg);
	}
	lseek(fd, st.st_size, SEEK_SET);
    } else len =  st.st_size;

    return len;
}

#define CRC32_BLOCK(buf, len) \
	fwh->csum = crc32_block_endian1(fwh->csum, \
		(uint8_t*)buf, len, fwh->crc32_table)

// FIXME: care endianess

int fw_print(fw_hdr* fwh)
{
    fw_comp* fwc;

    fdprintf(STDOUT_FILENO, "\nFirmware: %08x, %s\n",
	    fwh->flen + sizeof(fwh->csum), bb_basename(fwh->path));
    if (0) fdprintf(STDOUT_FILENO, "  Magic-num: %08x\n", fwh->magic);
    fdprintf(STDOUT_FILENO, "  Timestamp: %s",
	    asctime(localtime((time_t*)&fwh->stamp)));
    fdprintf(STDOUT_FILENO, "  Version: %d.%d.%d.%d, %08x\n",
	    fwh->vers.major, fwh->vers.minor, fwh->vers.rev,
	    fwh->vers.xx_, fwh->csum);
    fdprintf(STDOUT_FILENO, "  Vendor: %s\n", fwh->vendor);
    if (fwh->sign && fwh->sign[0])
    fdprintf(STDOUT_FILENO, "  Signature: %s\n", fwh->sign);
    fdprintf(STDOUT_FILENO, "  Components: %s\n", fwh->first ? "" : "<NONE>");

    for (fwc = fwh->first; fwc; fwc = fwc->next) {
	 fwc_info info = fwc->info;

	fdprintf(STDOUT_FILENO, "    [%08x]: %08x + %8x\n",
		info._,  fwc->offs, fwc->size);
    }	full_write1_str("\n");

    return 0;
}

int fw_open(fw_hdr* fwh)
{
    int fd;
    uint8_t n;

    if (!fwh->path) {
	const char* msg = "No firmware path!\n";
	if (0) full_write2_str(msg); else
	bb_error_msg_and_die(msg);
	return EINVAL;
    }

    if (fwh->stamp & 0x01) {	unlink(fwh->path);	// XXX:
	
	if ((fd = xopen3(fwh->path, O_WRONLY | O_CREAT | O_TRUNC,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) return errno;

	fwh->magic = FIRMWARE_MAGIC_NUMBER;
	fwh->stamp = time(NULL);

	if (!fwh->vers._) fwh->vers.major = 0, fwh->vers.minor = 1,
			  fwh->vers.rev = 0;
	if (!fwh->vendor) fwh->vendor = /*strdup*/(char*)(FIRMWARE_VENDOR);
	if (!fwh->sign) fwh->sign = /*strdup*/(char*)("");

	fwh->csum = 0;	fwh->crc32_table = crc32_filltable(NULL, 1);

	xwrite(fd, (char*)fwh, n = offsetof(fw_hdr, vendor));
	CRC32_BLOCK(fwh, n);

	n = strlen(fwh->vendor); xwrite(fd, &n, 1);
	xwrite(fd, fwh->vendor, n);

	CRC32_BLOCK(&n, 1);
	CRC32_BLOCK(fwh->vendor, n);

	n = strlen(fwh->sign);	 xwrite(fd, &n, 1);
	xwrite(fd, fwh->sign, n);

	CRC32_BLOCK(&n, 1);
	CRC32_BLOCK(fwh->sign, n);

	//fwh->flen = xlseek(fd, 0, SEEK_CUR);	// XXX:
    } else {
	fw_comp *last;
	uint32_t size, wr = fwh->stamp & 0x02;

	if ((fd = xopen(fwh->path, wr ? O_RDWR : O_RDONLY)) < 0) return errno;

	fwh->flen = fdlength(fd) - sizeof(fwh->csum);
	xread(fd, (char*)fwh, n = offsetof(fw_hdr, vendor));

	if (fwh->magic != FIRMWARE_MAGIC_NUMBER) {
	    const char* msg = "Corrupted firmware!\n";
	    if (0) full_write2_str(msg); else
	    bb_error_msg_and_die(msg);
	    return EINVAL;
	}

	n = 0;	xread(fd, &n, 1);
	xread(fd, fwh->vendor = xmalloc(n + 1), n);
	fwh->vendor[n] = '\0';

	n = 0;	xread(fd, &n, 1);
	xread(fd, fwh->sign   = xmalloc(n + 1), n);
	fwh->sign[n] = '\0';

	fwh->first = last = NULL;
	while ((size = xlseek(fd, 0, SEEK_CUR)) < fwh->flen) {
	    fw_comp* fwc = xmalloc(sizeof(*last));
	    xread(fd, (char*)fwc, n = offsetof(fw_comp, offs));

	    if (fwc->info.deflate) fwc->flen = fwc->size; else {
		xlseek(fd, fwc->size - 4, SEEK_CUR); // XXX: gzip
		xread(fd, (char*)&fwc->flen, 4);
	    }	fwc->offs = size + n;

	    if (last) last->next = fwc; last = fwc;
	    if (!fwh->first) fwh->first = last;
	}   xread(fd, (char*)&fwh->csum, sizeof(fwh->csum));

	if (wr) ftruncate(fd, fwh->flen);
    }

    fwh->fd = fd;	return 0;
}

int fw_check(fw_hdr* fwh)
{
    uint8_t* buf;
    uint32_t csum, offs;
    uint32_t len, per, total;
    
    if (fwh->fd < 1) return EBADF;

    len = sizeof(bb_common_bufsiz1) & ~0x07;
    buf = (uint8_t*)bb_common_bufsiz1;
    csum = fwh->csum; fwh->csum = 0;
    per = total = 0;

    full_write1_str("Integrity checking...\n");

    fwh->crc32_table = crc32_filltable(NULL, 1);
    offs = lseek(fwh->fd, 0, SEEK_SET);

    while (1) {
	int n = safe_read(fwh->fd, buf, len);
	if (n < 0) {
	    const char* msg = "read failed";
	    if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		    msg, strerror(errno)); else
	    bb_simple_perror_msg(msg);
	    return errno;
	} else
	if (n < 1) {
	    full_write1_str("\r100%\r");	break;
	}

	if (fwh->flen < (total += n))
	    n -= total - fwh->flen;
	CRC32_BLOCK(buf, n);

	if (per < (n = total * 100 / fwh->flen))
	    fdprintf(STDOUT_FILENO, "\r%3u%%", per = n);
    }	//full_write1_str("\n");

    lseek(fwh->fd, offs, SEEK_SET);

    CRC32_BLOCK(&fwh->flen, sizeof(fwh->flen));

    fwh->csum = ~fwh->csum;
    free(fwh->crc32_table);

    if (fwh->csum != csum) {
	const char* fmt = "Data corrupted: %08x vs %08x\n";
	if (0) fdprintf(STDERR_FILENO, fmt, fwh->csum, csum); else
	bb_error_msg(fmt, fwh->csum, csum);
	return EIO;
    }	full_write1_str("Exactly!\n");

    return 0;
}

fw_comp* fw_find(fw_hdr* fwh, fwc_info info)
{
    fw_comp* fwc;
    const char* fmt;

    if (fwh->fd < 1) return NULL;

    for (fwc = fwh->first; fwc; fwc = fwc->next)
	if (fwc->info.part == info.part) return fwc;

    fmt = "Missing component: %08x\n";
    if (0) fdprintf(STDERR_FILENO, fmt, info._); else
    bb_error_msg(fmt, info._);

    return NULL;
}

int fw_append(fw_hdr* fwh, fw_comp* fwc)
{
    int fd;
    char key[8];
    uint8_t* buf;
    fw_comp* tmp;
    uint32_t len, per, total;

    if (fwh->fd < 1) return EBADF;

    if (!fwc->path) {
	const char* msg = "No component path!\n";
	if (0) full_write2_str(msg); else
	bb_error_msg(msg);
	return EINVAL;
    }

    if (access( fwc->path, R_OK)) {
	if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		fwc->path, strerror(errno)); else
	bb_simple_perror_msg(fwc->path);
	return errno;
    }

    if ((fd = open_or_warn(fwc->path, O_RDONLY)) < 0) return errno;
    fdprintf(STDOUT_FILENO, "Appending: %s\n", fwc->path);

    fwc->size = fdlength(fd);
    len = offsetof(fw_comp, offs);
    fwc->offs = lseek(fwh->fd, 0, SEEK_END) + len;

    full_write(fwh->fd, (char*)fwc, len);
    CRC32_BLOCK(fwc, len);

    per = total = 0;
    len = sizeof(bb_common_bufsiz1) & ~0x07;
    buf = (uint8_t*)bb_common_bufsiz1;

    memcpy(key, (char*)fwh, 8);
    des_setparity(key);

    while (1) {
	int n = full_read(fd, buf, len);
	if (errno < 0) {
	    const char* msg = "read failed";
	    if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		    msg, strerror(errno)); else
	    bb_simple_perror_msg(msg);
	    return errno;
	} else if (n < 1) {
	    full_write1_str("\r100%\r");	break;
	}

	if (total < n) {	// XXX:
	    int s = ecb_crypt(key, (char*)buf,
		    n & ~0x07, DES_SW | DES_ENCRYPT);

	    if (DES_FAILED(s)) {
		const char* msg = "Encryption failed!\n";
		if (0) full_write2_str(msg); else
		bb_error_msg_and_die(msg);
		return DESERR_BADPARAM;
	    }
	}

	total += n;
	CRC32_BLOCK(buf, n);
	full_write(fwh->fd, buf, n);

	if ((n = fwc->size - total) < len) len = n;

	if (per < (n = total * 100 / fwc->size))
	    fdprintf(STDOUT_FILENO, "\r%3u%%", per = n);
    }	//full_write1_str("\n");

    fdprintf(STDOUT_FILENO, "Component [%08x]: %08x + %8x\n",
	    fwc->info._, fwc->offs, fwc->size);

    close(fd);

    if (!(tmp = malloc_or_warn(sizeof(*tmp)))) return ENOMEM;

    memcpy(tmp, fwc, sizeof(*fwc));
    fwc =  tmp;	fwc->next = NULL;

    if ((tmp = fwh->first)) {
	while (tmp->next) tmp = tmp->next;  tmp->next = fwc;
    } else fwh->first = fwc;

    return 0;
}

int fw_output(fw_hdr* fwh, fw_comp* fwc)
{
    int fd;
    char key[8];
    uint8_t* buf;
    uint32_t len, per, total;

    if (fwh->fd < 1) return EBADF;

    if ((fd = fwc->fd) & ~0xff) {	// XXX:
	if (access(fwc->path, W_OK))
	     fd = open3_or_warn(fwc->path, O_WRONLY | O_CREAT | O_TRUNC,
		    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); else
	     fd = open_or_warn (fwc->path, O_WRONLY);
	if  (fd < 0) return errno;

	if (!strncmp(fwc->path, "/dev/", 5)) {
	    uint64_t size;

	    if (!ioctl(fd, BLKGETSIZE64, &size) && size < fwc->flen) {
		const char* fmt = "Less capacity: %s\n";
		if (0) fdprintf(STDERR_FILENO, fmt, fwc->path); else
		bb_error_msg(fmt, fwc->path);
		return EIO;
	    }
	}

	fdprintf(STDOUT_FILENO, "Outputing: %s\n", fwc->path);
    }

    lseek(fwh->fd, fwc->offs, SEEK_SET);

    if (!fwc->info.deflate/* && XXX: not gzipped */) {
	const char* msg = "fd duplicate failed";
	struct fd_pair fdp;
	pid_t pid;

	if (piped_pair(fdp) < 0 || (pid = vfork()) == (pid_t)-1) {
	    const char* err = "pipe/vfork failed";
	    if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		    err, strerror(errno)); else
	    bb_simple_perror_msg(err);
	    return errno;
	}

	if (pid == 0) {
	    char* argv[] = { (char*)"gzip", (char*)"-cd", (char*)"-", NULL };

	    close(fdp.wr);

	    if (dup2(fd, 1) != 1 || dup2(fd, 0) != 0) {
		if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
			msg, strerror(errno)); else
		bb_simple_perror_msg(msg);
		_exit(errno);
	    }

	    BB_EXECVP_or_die(argv);
	}

	close(fdp.rd);
	if (dup2(fdp.wr, fd) != fd) {
	    if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		    msg, strerror(errno)); else
	    bb_simple_perror_msg(msg);
	    return errno;
	}
    }

    per = total = 0;
    len = sizeof(bb_common_bufsiz1) & ~0x07;
    buf = (uint8_t*)bb_common_bufsiz1;

    memcpy(key, (char*)fwh, 8);
    des_setparity(key);

    while (1) {
	int n = full_read(fwh->fd, buf, len);
	if (errno < 0) {
	    const char* msg = "read failed";
	    if (0) fdprintf(STDERR_FILENO, "%s: %s\n",
		    msg, strerror(errno)); else
	    bb_simple_perror_msg(msg);
	    return errno;
	} else if (n < 1) {
	    full_write1_str("\r100%\r");	break;
	}

	if (total < n) {	// XXX:
	    int s = ecb_crypt(key, (char*)buf,
		    n & ~0x07, DES_SW | DES_DECRYPT);

	    if (DES_FAILED(s)) {
		const char* msg = "Decryption failed!\n";
		if (0) full_write2_str(msg); else
		bb_error_msg_and_die(msg);
		return DESERR_BADPARAM;
	    }

	    if (fwc->info.part < ROOTFS && !strncmp(fwc->path, "/dev/", 5)) {
		// FIXME: must be signed?
	    }
	}

	full_write(fd, buf, n);  total += n;

	if ((n = fwc->size - total) < len) len = n;

	if (per < (n = total * 100 / fwc->size))
	    fdprintf(STDOUT_FILENO, "\r%3u%%", per = n);
    }	//full_write1_str("\n");

    fdprintf(STDOUT_FILENO, "Component: [%08x], %08x -> %08x\n",
	    fwc->info._, fwc->size, fwc->flen);

    fdatasync(fd);	fsync(fd);
    if (255 < (fwc->fd)) close(fd);

    return 0;
}

int fw_close(fw_hdr* fwh)
{
    fw_comp* fwc;

    if (fwh->fd < 1) return EBADF;

    if (fwh->flen < 1) { // XXX:
	uint32_t csum = fwh->csum;

	fwh->flen = xlseek(fwh->fd, 0, SEEK_END);
	CRC32_BLOCK(&fwh->flen, sizeof(fwh->flen));

	fwh->csum = ~fwh->csum;
	xwrite(fwh->fd, (char*)&fwh->csum, sizeof(fwh->csum));

	fw_print(fwh);	fwh->flen += 4;

	fwh->csum = crc32_block_endian1(csum, (uint8_t*)&fwh->csum,
		sizeof(fwh->csum), fwh->crc32_table);
	for (; fwh->flen; fwh->flen >>= 8)
	    CRC32_BLOCK(&fwh->flen, 1);

	fwh->csum = ~fwh->csum;
	free(fwh->crc32_table);
    }

    for (fwc = fwh->first; fwc; ) {
	fw_comp* next = fwc->next;
	free(fwc);	fwc = next;
    }

  if (0) {
    free((char*)fwh->path);
    free(fwh->vendor);
    free(fwh->sign);
  }

    close(fwh->fd);

    if (fwh->flen < 1) {
	char *path, *p;

	fwh->flen = strlen(fwh->path);

	if (0 && (path = fwh->path) && (path = strrchr(path, '/')) &&
	    (path = strchr(path, '-'))) fwh->flen -= (path - fwh->path);

	strncpy(path = xmalloc(fwh->flen + 40), fwh->path, fwh->flen);

	fwh->flen += sprintf( &path[fwh->flen], "-v%u.%u.%u",
		fwh->vers.major, fwh->vers.minor, fwh->vers.rev);

	if (0)
	fwh->flen += strftime(&path[fwh->flen], 16, "-%y%m%d",
		localtime((time_t*)&fwh->stamp));

	fwh->flen += sprintf( &path[fwh->flen], "-%08x", fwh->csum);

	p = (char*)bb_basename(fwh->path);
	fdprintf(STDOUT_FILENO, "Renaming: %s -> %s\n",
		p, path + (p - fwh->path));

	xrename(fwh->path, path);
	symlink(path, fwh->path);

	free(path);
    }

    return 0;
}

// vim:sts=4:ts=8:
