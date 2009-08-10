/****************************************************************
 * $ID: fwutil.h       Tue, 17 Mar 2009 10:19:09 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyLeft (c)  2009  M.H.Fan                                  *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/
#ifndef FWUTIL_H
#define FWUTIL_H

enum {
    FWH = 0,

    QI, XLOADER = QI, TCBOOT = QI, SPL = QI,

    UBOOT, BOOTLOADER = UBOOT,

    UIMAGE, ZIMAGE = UIMAGE,
    INITRD, INITRAMFS = INITRD, RAMDISK = INITRD,

    ROOTFS,
    USERFS, HOMEFS = USERFS,

    SWAPFS,

    DROID_RECOVERY,

    DROID_KERNEL,	// XXX: put it in RAMDISK/SYSTEM image?
    DROID_RAMDISK,

    DROID_USERDATA,
    DROID_SYSTEM,
    DROID_CACHE,

    FIRMWARE_MAGIC_NUMBER = ' FMH',
#define FIRMWARE_VENDOR "M.H.Fan"
};

typedef union { uint32_t _;
    struct { uint32_t part:8, deflate:1, crypto:4, type:4, nsec:8; };
}   fwc_info;

typedef struct __attribute__((packed)) fw_comp {
    fwc_info info;
    uint32_t size;

    // XXX: content

    uint32_t offs;
    uint32_t flen;

    union {
	int fd;
	const char* path;
    };

    struct fw_comp* next;
}   fw_comp;

typedef struct __attribute__((packed)) fw_hdr {
    uint32_t magic;	// 'hhcn'
    uint32_t stamp;	// seconds since the Epoch, XXX: time_t

    union { uint32_t _;
	struct {
	    uint8_t major, minor, rev, xx_;
	};
    }	vers;

    char* vendor;
    char* sign;

    uint32_t  csum;	// locate in the end of the file
    uint32_t* crc32_table;

    const char* path;
    uint32_t flen;
    int fd;

    fw_comp* first;
}   fw_hdr;

int fw_open(fw_hdr* fwh);
fw_comp* fw_find(fw_hdr* fwh, fwc_info info);
int fw_append(fw_hdr* fwh, fw_comp* fwc);
int fw_output(fw_hdr* fwh, fw_comp* fwc);
int fw_check(fw_hdr* fwh);
int fw_print(fw_hdr* fwh);
int fw_close(fw_hdr* fwh);

#endif//FWUTIL_H
// vim:sts=4:ts=8:
