#!/usr/bin/make -f
 ################################################################
 # $ID: Makefile       Thu, 31 Jul 2008 09:28:21 +0800  mhfan $ #
 #                                                              #
 # Description:                                                 #
 #                                                              #
 # Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            #
 #                                                              #
 # CopyRight (c)  2009~2010  M.H.Fan                            #
 #   All rights reserved.                                       #
 ################################################################

CC	 := arm-linux-gnu-gcc
CC	 := arm-linux-gnueabi-gcc
CC	 := arm-none-linux-gnueabi-gcc
CC	 := gcc

CONFIG_MMSTCAR := y
-include Kbuild

CPPFLAGS += -DNDEBUG -DCONFIG_PROPERTY  -D_GNU_SOURCE  #-DDEBUG -DSANITY_WRAP
CFLAGS   += -Wall -pipe -O3 $(EXTRA_CFLAGS)
LDLIBS   += -lnmea
LDFLAGS  +=

all: mmstcar

mmstcar: $(lib-y)

 # vim:sts=4:ts=8:
