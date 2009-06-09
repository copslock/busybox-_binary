#!/bin/bash
 ################################################################
 # $ID: make-linux.sh  Fri, 24 Feb 2006 17:29:48 +0800  mhfan $ #
 #                                                              #
 # Description:                                                 #
 #                                                              #
 # Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@hhcn.com>            #
 #                                                              #
 # CopyLeft (c)  2006~2009  HHTech                              #
 #   All rights reserved.                                       #
 #                                                              #
 # This file is free software;                                  #
 #   you are free to modify and/or redistribute it  	        #
 #   under the terms of the GNU General Public Licence (GPL).   #
 ################################################################

CFLAGS=
LDFLAGS=


ARCH=mips
SUBARCH=jz47xx
CFLAGS="-march=mips32"


ARCH=microblaze
SUBARCH=
CFLAGS=


ARCH=arm

SUBARCH=imx

SUBARCH=s3c24xx
CFLAGS="-march=armv4t -mtune=arm9tdmi"

SUBARCH=davinci
CFLAGS="-march=armv4 -mtune=arm926ej-s"

SUBARCH=s3c64xx
CFLAGS="-march=armv6zk -mtune=arm1176jzf-s"

SUBARCH=omap3
CFLAGS="-march=armv7-a -mtune=cortex-a8"

SUBARCH=s5pv210
CFLAGS="-march=armv7-a -mtune=cortex-a8"

SUBARCH=omap4
CFLAGS="-march=armv7-a -mtune=cortex-a8"	# XXX: cortex-a9

#CFLAGS="$CFLAGS -meabi=4 -mfloat-abi=soft"
#CFLAGS="$CFLAGS -mabi=aapcs-linux -mno-thumb-interwork -msoft-float"
CFLAGS="$CFLAGS -mabi=aapcs-linux -mno-thumb-interwork"	# XXX: -mthumb

#CFLAGS="$CFLAGS -mabi=apcs-gnu -mno-thumb-interwork -marm"

CFLAGS="$CFLAGS -fsingle-precision-constant -ftree-vectorize"
CFLAGS="$CFLAGS -fpromote-loop-indices -funroll-loops"	# XXX:

CFLAGS="$CFLAGS -mfloat-abi=softfp" # softfp|hard
#CFLAGS="$CFLAGS -mfpu=neon" # XXX: vfp|neon


CROSS_COMPILE=${CROSS_COMPILE:-arm-none-linux-gnueabi-}
CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnueabi-}
CROSS_COMPILE=${CROSS_COMPILE:-arm-linux-gnu-}

CROSS_COMPILE=${CROSS_COMPILE:-mipsel-linux-}
CROSS_COMPILE=${CROSS_COMPILE:-mipsel-linux-gnu-}

JOBS=$(grep -m 1 'cpu cores' /proc/cpuinfo | cut -d: -f2)

#export PATH=/opt/mipseltools-gcc412-lnx26/bin:$PATH
export PATH=/opt/arm-sourcery/bin:$PATH

PREFIX=${PREFIX:-../staging}
CONFIG_PREFIX=${CONFIG_PREFIX:-../rootfs}
CONFIG_PREFIX=${CONFIG_PREFIX:-$PREFIX}
INSTALL_PATH=${INSTALL_PATH:-../firmware}

INSTALL_MOD_PATH=${INSTALL_MOD_PATH:-$CONFIG_PREFIX}
CONFIG_INITRAMFS_SOURCE=${CONFIG_INITRAMFS_SOURCE:-$INSTALL_MOD_PATH}
export PREFIX INSTALL_PATH INSTALL_MOD_PATH CONFIG_PREFIX
export CONFIG_INITRAMFS_SOURCE

CFLAGS="$CFLAGS -Wall -pipe -D_GNU_SOURCE=1 -DNDEBUG=1 -O2"
CFLAGS="$CFLAGS -ffast-math -fomit-frame-pointer"

if echo $PWD | grep -qvs linux; then
    CFLAGS="$CFLAGS -I$PREFIX/include"
    LDFLAGS="$LDFLAGS -L$PREFIX/lib" # -Wl,--rpath-link -Wl,$PREFIX/lib
fi

export CROSS_COMPILE CFLAGS LDFLAGS

copy_lib() {
    dir=`${CROSS_COMPILE}gcc $CFLAGS -print-file-name=libc.a`
    #dir=`dirname $dir`/../../lib
    dir=`dirname $dir`

    echo $dir;
    cp -a $dir/*.so* $INSTALL_MOD_PATH/lib/
    for f in $INSTALL_MOD_PATH/{bin,lib}/*; do
	[ -f $f ] && ${CROSS_COMPILE}strip -s -R .comment -R .note $f;
    done
}

#copy_lib; exit 0
\make ARCH=$ARCH SUBARCH=$SUBARCH CONFIG_PREFIX=$CONFIG_PREFIX \
	CROSS_COMPILE=${CROSS_COMPILE} $@ \
#	CONFIG_INITRAMFS_SOURCE=$CONFIG_INITRAMFS_SOURCE \

[ -e $CONFIG_INITRAMFS_SOURCE/bin/busybox ] && \
echo "$@" | grep -qs modules_install && [ -e arch/arm ] && \
echo -n "Generating initramfs: " && (cd "$CONFIG_INITRAMFS_SOURCE" && \
find . | cpio -o -H newc) | gzip -9 -n > arch/$ARCH/boot/rootfs.cpio.gz && \
mkimage -A $ARCH -O linux -C gzip -T ramdisk -n 'Compressed initial ramdisk' \
	-d arch/$ARCH/boot/rootfs.{cpio.gz,uboot} && \
rm -f arch/$ARCH/boot/rootfs.cpio.gz

which ${CROSS_COMPILE}gcc
#dirname `${CROSS_COMPILE}gcc $CFLAGS -print-file-name=libc.a`

 ############### End Of File: make-linux.sh ################
