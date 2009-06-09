#!/usr/bash
 ################################################################
 # $Id: hhtech_config.sh	     2007-08-16 11:08:46Z svn $ #
 #                                                              #
 # Description:							#
 #                                                              #
 # Maintainer:  ∑∂√¿ª‘(Meihui FAN)  <mhfan@hhcn.com>            #
 #                                                              #
 # CopyRight (c)  2006~2009  HHTech                             #
 #   www.hhcn.com, www.hhcn.org                                 #
 #   All rights reserved.                                       #
 #                                                              #
 # This file is free software;                                  #
 #   you are free to modify and/or redistribute it  	        #
 #   under the terms of the GNU General Public Licence (GPL).   #
 ################################################################

ARCH=${ARCH:-$(uname -m)}

if [ -z $ARCH ]; then
    if   echo $PWD $0 $1 | grep -q  x86; then ARCH=x86;
    elif echo $PWD $0 $1 | grep -q  arm; then ARCH=arm; #ARCH=armv4
    elif echo $PWD $0 $1 | grep -q bfin; then ARCH=bfin;
    elif echo $PWD $0 $1 | grep -q mipsel; then ARCH=mipsel;
    elif echo $PWD $0 $1 | grep -q mips; then ARCH=mips;
    else echo "I recommand that make an another directory whose name contains your target architecture name, and then run lndir against this source directory! Then you can do anthing what you want in that directory. e.g.:	mkdir ../build-bfin; lndir `pwd` ../build-bfin"; exit 0;
    fi
fi

PREFIX=${PREFIX:-/usr}
#PREFIX=${PREFIX:-/home/mhfan/devel/staging/${ARCH}}
#export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"

unset CC CFLAGS CXX CXXFLAGS CPPFLAGS LDFLAGS LDLIBS LIBS

if [ ${ARCH} = bfin ]; then

    UCLINUX=${UCLINUX:-/home/mhfan/devel/hhbf/BF548}
    PREFIX=${UCLINUX}/staging/usr

    TARGET=${TARGET:-bfin-linux-uclibc}
    TARGET=${TARGET:-bfin-uclinux}

    which $TARGET-gcc #|| export PATH=/opt/bfin-uclinux/bin:$PATH

    CPU=${CPU:-bf548-0.1}
    CPU=${CPU:-bf533-0.5}
    CPU=${CPU:-bf561-0.3}

    OPTFLAGS="-O2 -mfast-fp -mcpu=${CPU}"

    LIBS=""

    LDFLAGS=""

    LDFLAGS="${LDFLAGS} -mfast-fp"
    CPPFLAGS="-D__uClinux__ -DEMBED"

    if [ ${TARGET} = bfin-linux-uclibc ]; then
	OPTFLAGS="-mfdpic $OPTFLAGS";
	LDFLAGS=" -mfdpic $LDFLAGS"	# XXX: -shared
	LDFLAGS="$LDFLAGS -Wl,--defsym,__stacksize=0x8000"
    else
	LDFLAGS="$LDFLAGS -Wl,-elf2flt=\"-s 0x4000\""	# -static
    fi

    LIBS="${LIBS} -lbffastfp -lbfdsp -lm"	# XXX:

    if [ ! -e ${UCLINUX} ]; then
	echo "Missing uClinux-dist directory path:	${UCLINUX}"; sleep 1;
    fi

    #export STAGEDIR=${UCLINUX}/staging;
    #export PKG_CONFIG=${UCLINUX}/tools/${TARGET}-pkg-config;
elif [ ${ARCH} = armdroid ]; then
    TARGET_ARCH_ABI=${TARGET_ARCH_ABI:-armeabi}
    ANDROID_NDK=${ANDROID_NDK:-/home/mhfan/devel/mhdroid/ndk}
    ANDROID_USR=${ANDROID_USR:-${ANDROID_NDK}/platforms/android-9/arch-arm/usr}
    GNUSTDCXX=${ANDROID_NDK}/sources/cxx-stl/gnu-libstdc++

    TARGET=${TARGET:-arm-linux-androideabi}
    TARGET=${TARGET:-arm-eabi}

    export PKG_CONFIG_LIBDIR="${PREFIX}/lib/pkgconfig"
    which $TARGET-gcc || export PATH=${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin:$PATH

    LIBS="-lc -lgcc"
    CFLAGS="-std=c99"	# XXX:
    CPPFLAGS="--sysroot ${ANDROID_USR} -I =/include"	# -iprefix
    LDFLAGS="-Wl,-rpath-link=${ANDROID_USR}/lib -L${ANDROID_USR}/lib"
    LDFLAGS="${LDFLAGS} -Wl,-Bdynamic,-dynamic-linker=/system/bin/linker"
    LDFLAGS="${LDFLAGS} -Wl,--no-undefined -Wl,-shared -nostdlib"

    CXXFLAGS="-fexceptions -frtti"	# XXX:
    CXXFLAGS="${CXXFLAGS} -I${GNUSTDCXX}/include"
    CXXFLAGS="${CXXFLAGS} -I${GNUSTDCXX}/lib/${TARGET_ARCH_ABI}/include"
    CXXLIBS="${GNUSTDCXX}/libs/${TARGET_ARCH_ABI}/libstdc++.a"
elif [ ${ARCH/arm*/arm} = arm ]; then

    #ARM_ARCH=${ARCH}v4
    #ARM_ARCH=${ARCH}v4t
    ARM_ARCH=${ARCH}v6zk
    #ARM_ARCH=${ARCH}v7-a

    #CPU=${CPU:-cortex-a8}
    CPU=${CPU:-arm1176jzf-s}
    CPU=${CPU:-arm926ej-s}
    CPU=${CPU:-arm9tdmi}
    #CPU=${CPU:-arm}

    TARGET=${TARGET:-arm-none-linux-gnueabi}
    TARGET=${TARGET:-arm-linux-gnueabi}
    TARGET=${TARGET:-arm-linux-gnu}

    which $TARGET-gcc || export PATH=/opt/arm-sourcery/bin:$PATH

    LIBS=""

    LDFLAGS="-static"
    LDFLAGS=""

    OPTFLAGS="-O2 -march=${ARM_ARCH} -mtune=${CPU}"	# XXX: -mthumb
    OPTFLAGS="${OPTFLAGS} -mno-thumb-interwork" # -mabi=aapcs-linux	# XXX:
    #OPTFLAGS="${OPTFLAGS} -mabi=apcs-gnu -mno-thumb-interwork -mno-apcs-frame"
    OPTFLAGS="${OPTFLAGS} -fsingle-precision-constant -ftree-vectorize"
    OPTFLAGS="${OPTFLAGS} -fpromote-loop-indices -funroll-loops"
    #OPTFLAGS="${OPTFLAGS} -ftree-vectorize-verbose=1"
    OPTFLAGS="${OPTFLAGS} -mfloat-abi=softfp" # softfp|hard
#    OPTFLAGS="${OPTFLAGS} -mfpu=neon" # vfp|neon

elif [ ${ARCH/mips*/mips} = mips ]; then

    LIBS=""

    LDFLAGS=""

    OPTFLAGS="-O2 -march=mips32" #-msoft-float

    TARGET=${TARGET:-mipsel-linux}
    TARGET=${TARGET:-mipsel-linux-gnu}

    which $TARGET-gcc || export PATH=/opt/mipseltools-gcc412-lnx26/bin:$PATH

elif [ ${ARCH/*86/x86} = x86 ]; then	CPU=${CPU:-core2};	# k8
    OPTFLAGS="-O2 -mtune=$CPU"	# -march=athlon-xp
fi

### XXX: you don't need to modify the following lines.

#TARGET=$(gcc -dumpmachine)
if [ ${ARCH/*86/x86} = x86 ]; then CROSS_COMPILE=; else
    CROSS_COMPILE=${CROSS_COMPILE:+${TARGET}-}
fi

export CPU ARCH TARGET CROSS_COMPILE

CPPFLAGS="${CPPFLAGS} -DCONFIG_HHTECH=1 -D_GNU_SOURCE=1 -DNDEBUG=1"
OPTFLAGS="${OPTFLAGS} -fomit-frame-pointer -ffast-math -fsigned-char"

#if [ -n ${PREFIX} -a -e ${PREFIX} ]; then
CPPFLAGS="${CPPFLAGS} -isystem ${PREFIX}/include"	# -B
LDFLAGS="${LDFLAGS} -Wl,--as-needed -L${PREFIX}/lib"
LDFLAGS="${LDFLAGS} -Wl,--rpath-link=${PREFIX}/lib"
#fi

CFLAGS="-Wall -pipe ${OPTFLAGS} ${CFLAGS}"	# XXX: ${CPPFLAGS}

ASFLAGS=" ${CFLAGS} ${ASFLAGS}"

CXXFLAGS="${CFLAGS} ${CXXFLAGS}"

CXXLIBS="${LIBS} ${CXXLIBS}"

LDLIBS=" ${LIBS} ${LDLIBS}"

export CFLAGS CPPFLAGS LIBS LDLIBS LDFLAGS ASFLAGS CXXFLAGS CXXLIBS

AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
NM=${CROSS_COMPILE}nm
CC=${CROSS_COMPILE}gcc
CPP=${CROSS_COMPILE}cpp
CXX=${CROSS_COMPILE}g++
CXXCPP=${CROSS_COMPILE}cpp
STRIP="${CROSS_COMPILE}strip -s -R .comment -R .note"

if [ ${ARCH} = bfin ]; then
CC=${CROSS_COMPILE}gcc-4.1.2
CXX=${CROSS_COMPILE}g++-4.1.2
fi # XXX:

export AS LD CC CXX CPP CXXCPP

HOST=i486-linux-gnu;
HOSTCC=gcc; HOSTCXX=g++;

export HOST HOSTCC HOSTCXX

export _PREFIX=${CONFIG_PREFIX:-$PREFIX}

if [ -f local_config.sh ]; then source local_config.sh; fi

#CC="$CC" \
#LD="$LD" \
#AS="$AS" \
#NM="$NM" \
#CPP="$CPP" \
#CXX="$CXX" \
#LIBS="$LIBS" \
#CFLAGS="$CFLAGS" \
#LDFLAGS="$LDFLAGS" \
#CXXLIBS="$CXXLIBS" \
#CPPFLAGS="$CPPFLAGS" \
#CXXFLAGS="$CXXFLAGS" \
#STRIP="$STRIP" \
#./configure $CONFIG_ARGS \
#  --prefix=$PREFIX \

# vim:sts=4:ts=8:
