#!/usr/bin/make -f
 ################################################################
 # $Id: hhtech_config.mak 1468 2007-08-16 11:08:46Z svn $ #
 #                                                              #
 # Description:							#
 #                                                              #
 # Maintainer:  ∑∂√¿ª‘(Meihui Fan)  <mhfan@hhcn.com>            #
 #                                                              #
 # CopyRight (c)  2006~2009  HHTech                             #
 #   www.hhcn.com, www.hhcn.org                                 #
 #   All rights reserved.                                       #
 #                                                              #
 # This file is free software;                                  #
 #   you are free to modify and/or redistribute it  	        #
 #   under the terms of the GNU General Public Licence (GPL).   #
 ################################################################

#.EXPORT_ALL_VARIABLES:

ARCH ?= $(shell uname -m)

PREFIX ?= /usr
#PREFIX ?= /home/mhfan/devel/staging/$(ARCH)
##PKG_CONFIG_PATH="$(PREFIX)/lib/pkgconfig:$(PKG_CONFIG_PATH)"

CFLAGS=
CXXFLAGS=
CPPFLAGS=
LDFLAGS=
LDLIBS=
LIBS=

#unset CC CFLAGS CXX CXXFLAGS CPPFLAGS LDFLAGS LDLIBS LIBS

ifeq ($(subst blackfin,bfin,$(ARCH)),bfin)

UCLINUX ?= /home/mhfan/devel/hhbf/BF548
PREFIX= $(UCLINUX)/staging/usr

ifdef	UTILIZE_UCLINUX
ROOTDIR= $(UCLINUX)
LIBCDIR= uClibc
UCLINUX_BUILD_USER= y
LINUXDIR= linux-2.6.x
-include $(ROOTDIR)/config/.config
-include $(ROOTDIR)/vendors/config/bfin/config.arch
ROMFSINST= $(ROOTDIR)/tools/romfs-inst.sh
ROMFSDIR=  $(ROOTDIR)/romfs
endif	# XXX: FIXME

TARGET ?= bfin-linux-uclibc
TARGET ?= bfin-uclinux

CPU ?= bf548-0.1
CPU ?= bf533-0.5
CPU ?= bf561-0.3

OPTFLAGS= -O2 -mfast-fp -mcpu=$(CPU) #

LIBS=

LDFLAGS=

LDFLAGS += -mfast-fp
CPPFLAGS= -D__uClinux__ -DEMBED

ifeq ($(TARGET),bfin-linux-uclibc)
OPTFLAGS += -mfdpic
LDFLAGS  += -mfdpic -Wl,--defsym,__stacksize=0x8000 # XXX: -shared
else
LDFLAGS  += -Wl,-elf2flt="-s 0x4000" #-static
endif

LIBS += -lbffastfp -lbfdsp -lm	# XXX:

#STAGEDIR= $(UCLINUX)/staging
#PKG_CONFIG= $(UCLINUX)/tools/$(TARGET)-pkg-config
else

ifeq ($(ARCH),armdroid)

TARGET_ARCH_ABI ?= armeabi
ANDROID_NDK ?= /home/mhfan/devel/mhdroid/ndk
ANDROID_USR ?= $(ANDROID_NDK)/platforms/android-9/arch-arm/usr
GNUSTDCXX= $(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++

TARGET ?= arm-eabi
TARGET ?= arm-linux-androideabi

#export PKG_CONFIG_LIBDIR="${PREFIX}/lib/pkgconfig"
#which $TARGET-gcc || export PATH=${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin:$PATH

LIBS= -lc -lgcc
CFLAGS= -std=c99	# XXX:
CPPFLAGS= --sysroot $(ANDROID_USR) -I =/include	# -iprefix
LDFLAGS= -Wl,-rpath-link=$(ANDROID_USR)/lib -L$(ANDROID_USR)/lib
LDFLAGS= $(LDFLAGS) -Wl,-Bdynamic,-dynamic-linker=/system/bin/linker
LDFLAGS= $(LDFLAGS) -Wl,--no-undefined -Wl,-shared -nostdlib

CXXFLAGS= -fexceptions -frtti	# XXX:
CXXFLAGS= $(CXXFLAGS) -I$(GNUSTDCXX)/include
CXXFLAGS= $(CXXFLAGS) -I$(GNUSTDCXX)/lib/$(TARGET_ARCH_ABI)/include
CXXLIBS= $(GNUSTDCXX)/libs/$(TARGET_ARCH_ABI)/libstdc++.a

else

ifeq ($(ARCH:arm%=arm),arm)

#ARM_ARCH= $(ARCH)v4
#ARM_ARCH= $(ARCH)v4t
ARM_ARCH= $(ARCH)v6zk
#ARM_ARCH= $(ARCH)v7-a

#CPU ?= cortex-a8
CPU ?= arm1176jzf-s
CPU ?= arm926ef-s
CPU ?= arm9tdmi
CPU ?= arm

TARGET ?= arm-none-linux-gnueabi
TARGET ?= arm-linux-gnueabi
TARGET ?= arm-linux-gnu

TARGET ?= arm-9tdmi-linux-gnu
TARGET ?= arm-elf-linux
TARGET ?= armv4l-unknown-linux
TARGET ?= arm-softfloat-linux-gnu

#which $TARGET-gcc || export PATH=/opt/arm-sourcery/bin:$PATH

LIBS=

LDFLAGS= -static
LDFLAGS=

OPTFLAGS= -O2 -march=${ARM_ARCH} -mtune=${CPU}	# XXX: -mthumb
OPTFLAGS += -mno-thumb-interwork #-mabi=aapcs-linux	# XXX:
#OPTFLAGS += -mabi=apcs-gnu -mno-thumb-interwork  -mno-apcs-frame
OPTFLAGS += -fsingle-precision-constant -ftree-vectorize
OPTFLAGS += -fpromote-loop-indices -funroll-loops
#OPTFLAGS += -ftree-vectorize-verbose=1
OPTFLAGS += -mfloat-abi=softfp # softfp|hard
#OPTFLAGS += -mfpu=neon # vfp|neon

else

ifeq ($(ARCH:%86=x86),x86)

#CPU ?= k8
CPU ?= core2
OPTFLAGS=-O2 -mtune=$(CPU)	# -march=athlon-xp

CROSS_COMPILE=

endif	# x86

endif	# arm

endif	# armdroid

endif	# bfin

### XXX: you don't need to modify the following lines.

#TARGET=$(shell gcc -dumpmachine)
CROSS_COMPILE ?= $(TARGET)-

export CPU ARCH TARGET CROSS_COMPILE

OPTFLAGS += -fomit-frame-pointer -ffast-math -fsigned-char

CPPFLAGS += -DCONFIG_HHTECH=1 -D_GNU_SOURCE=1 -DNDEBUG=1

ifneq ($(PREFIX),)

CPPFLAGS += -isystem $(PREFIX)/include	# -B
LDFLAGS  += -Wl,--as-needed -L$(PREFIX)/lib \
	    -Wl,--rpath-link=${PREFIX}/lib

endif

CFLAGS += -Wall -pipe $(OPTFLAGS) # XXX:

ASFLAGS  += ${CFLAGS}
CXXFLAGS += ${CFLAGS}

CXXLIBS += ${LIBS}
LDLIBS  += ${LIBS}

export CFLAGS CPPFLAGS LIBS LDLIBS LDFLAGS ASFLAGS CXXFLAGS CXXLIBS

AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
NM=$(CROSS_COMPILE)nm
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
CPP=$(CROSS_COMPILE)cpp
CXXCPP=$(CROSS_COMPILE)cpp
STRIP=$(CROSS_COMPILE)strip -s -R .comment -R .note

ifeq ($(ARCH),bfin)
CC=$(CROSS_COMPILE)gcc-4.1.2
CXX=$(CROSS_COMPILE)g++-4.1.2
endif # XXX:

export AS LD CC CXX CPP CXXCPP STRIP

HOSTCC=gcc
HOSTCXX=g++
HOST=i486-linux-gnu

export HOST HOSTCC HOSTCXX

_PREFIX=$(if $(CONFIG_PREFIX),$(CONFIG_PREFIX),$(PREFIX))

export _PREFIX

-include local_config.mak
# vim:sts=4:ts=8:
