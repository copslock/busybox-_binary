#!/bin/make -f
 ################################################################
 # $ID: Android.mk     Wed, 01 Sep 2010 14:19:27 +0800  mhfan $ #
 #                                                              #
 # Description:                                                 #
 #                                                              #
 # Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            #
 #                                                              #
 # CopyLeft (c)  2010  M.H.Fan                                  #
 #   All rights reserved.                                       #
 #                                                              #
 # This file is free software;                                  #
 #   you are free to modify and/or redistribute it  	        #
 #   under the terms of the GNU General Public Licence (GPL).   #
 ################################################################

LOCAL_PATH := $(call my-dir)


CROSS_COMPILE := $(CURDIR)/$(subst gcc,,$(word \
	$(words $(TARGET_CC)),$(TARGET_CC)))

ANDROID_CFLAGS  := $(TARGET_GLOBAL_CFLAGS) \
	-isysroot $(CURDIR) $(addprefix -I =/,"" $(TARGET_C_INCLUDES))

ANDROID_LDFLAGS := $(TARGET_GLOBAL_LDFLAGS) \
	$(addsuffix $(CURDIR)/$(TARGET_OUT_INTERMEDIATE_LIBRARIES), \
		-Wl,-rpath-link= -L) -nostdlib -lc \
	$(CURDIR)/$(TARGET_CRTBEGIN_DYNAMIC_O) \
        #-Wl,-dynamic-linker,/system/bin/linker -Wl,--gc-sections \
        #$(TARGET_FDO_LIB) $(TARGET_LIBGCC) $(TARGET_CRTEND_O) \
# XXX: build/core/combo/TARGET_linux-arm.mk


$(hide $(shell cd $(LOCAL_PATH) && (( \
	test configs/android_defconfig -nt .config && \
	$(MAKE) android_defconfig > /dev/null) || \
	test .config -nt busybox.links) && \
	$(MAKE) ARCH=$(TARGET_ARCH) prepare busybox.links))


BUSYBOX_OBJS :=
TOPD := $(TOPDIR)
ARCH := $(TARGET_ARCH)
BUILD_FOR_ANDROID := yes
KBUILD_SRC := $(LOCAL_PATH)

include $(LOCAL_PATH)/.config
include $(LOCAL_PATH)/Makefile

define COLLECT_OBJS

obj-y :=
#lib-y :=

include $(LOCAL_PATH)/$(1)/Kbuild

BUSYBOX_OBJS += $$(addprefix $(1)/,$$(lib-y) $$(obj-y))

endef

$(foreach D,$(busybox-dirs),$(eval $(call COLLECT_OBJS,$(D))))

TOPDIR := $(TOPD)

#BUSYBOX_SRC := $(shell $(MAKE) -s -C $(LOCAL_PATH) CC=$(CC) show-sources)
BUSYBOX_SRC := $(patsubst %.o,%.c,$(sort $(BUSYBOX_OBJS)))
BUSYBOX_SRC += libbb/android.c
#$(warning $(BUSYBOX_SRC))

BUSYBOX_INC :=

ifeq (1,1)
BUSYBOX_SRC += $(subst $(LOCAL_PATH)/,,$(wildcard \
	$(LOCAL_PATH)/android/libc/arch-$(TARGET_ARCH)/syscalls/*.S))

BUSYBOX_INC += bionic/libm/include bionic/libm \
	       bionic/libc/private bionic/libc/kernel/common
endif

BUSYBOX_INC += $(LOCAL_PATH)/include external/mtd-utils/include

BUSYBOX_CFLAGS := $(CPPFLAGS) $(CFLAGS) -D"KBUILD_STR(s)=\#s" \
	-Ibionic/libc/private -Wno-shadow -Wno-format -Wno-extra \
	-D__ISO_C_VISIBLE=1999 -D__XPG_VISIBLE=601 #-D__POSIX_VISIBLE=200112 \
	#-D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -D_FILE_OFFSET_BITS=64 \

BUSYBOX_CFLAGS += $(if $(wildcard $(LOCAL_PATH)/../mtd-utils/include/*),, \
		  -DANDROID_NO_MTD=1)
BUSYBOX_CFLAGS += -DBIONIC_ICS=1

include $(CLEAR_VARS)
LOCAL_MODULE := busybox_prepare
$(LOCAL_MODULE): $(LOCAL_PATH)/busybox.links

$(LOCAL_PATH)/busybox.links: $(LOCAL_PATH)/.config
	$(MAKE) -C $(LOCAL_PATH) ARCH=$(TARGET_ARCH) prepare busybox.links

$(LOCAL_PATH)/.config: $(LOCAL_PATH)/configs/android_defconfig
	$(MAKE) -C $(LOCAL_PATH) android_defconfig > /dev/null

LOCAL_MODULE_TAGS := eng
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libclearsilverregex

ifeq (0,1)
LOCAL_C_INCLUDES := external/clearsilver \
		    external/clearsilver/util/regex
LOCAL_SRC_FILES  := external/clearsilver/util/regex/regex.c
else
LOCAL_SRC_FILES  := android/regex/regex.c
LOCAL_C_INCLUDES := android/regex
endif

LOCAL_MODULE_TAGS := eng
include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := busybox
$(LOCAL_MODULE): busybox_prepare

LOCAL_SRC_FILES  := $(BUSYBOX_SRC)  android/libc/__set_errno.c
LOCAL_C_INCLUDES := $(BUSYBOX_INC)
LOCAL_CFLAGS := $(BUSYBOX_CFLAGS)

LOCAL_SHARED_LIBRARIES := liblog #libz #libclearsilver
LOCAL_STATIC_LIBRARIES := libclearsilverregex

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)


BUSYBOX_LINKS := $(notdir $(shell cat $(LOCAL_PATH)/busybox.links))
BUSYBOX_LINKS := $(addprefix $(LOCAL_MODULE_PATH)/, \
	$(filter-out su nc, $(BUSYBOX_LINKS)))	# XXX:

$(BUSYBOX_LINKS): BUSYBOX_BINARY := $(LOCAL_MODULE)
$(BUSYBOX_LINKS): $(LOCAL_INSTALLED_MODULE)
	@#[ -f '$@' ] && echo "$@ exist!"
	@echo "Symlink: $@ -> $(BUSYBOX_BINARY)"; \
		ln -sf $(BUSYBOX_BINARY) $@; #mkdir -p $(dir $@)

$(LOCAL_MODULE_PATH): ; mkdir -p $@
	@TMPV=$(TARGET_OUT)/usr; mkdir -p $$TMPV; TMPV=$$TMPV/bin; \
		[ -e $$TMPV ] || ln -sf ../xbin $$TMPV

ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE_PATH) $(BUSYBOX_LINKS) \

# We need this so that the installed files could be picked up
# based on the local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
	$(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) \
	$(LOCAL_MODULE_PATH) $(BUSYBOX_LINKS) \


include $(CLEAR_VARS)
LOCAL_MODULE := libbusybox
$(LOCAL_MODULE): busybox_prepare

LOCAL_SRC_FILES  := $(BUSYBOX_SRC)
LOCAL_C_INCLUDES := $(BUSYBOX_INC)

LOCAL_CFLAGS := $(BUSYBOX_CFLAGS) -Dmain=busybox_driver \
	-Dgenerate_uuid=bb_generate_uuid \
	# XXX: to avoid multiple definition in libext4_utils

LOCAL_STATIC_LIBRARIES := libclearsilverregex

LOCAL_MODULE_TAGS := eng
include $(BUILD_STATIC_LIBRARY)

ifeq (0,1)
include $(CLEAR_VARS)
LOCAL_MODULE := busybox_static
$(LOCAL_MODULE): busybox_prepare

LOCAL_SRC_FILES  := $(BUSYBOX_SRC)  android/libc/__set_errno.c
LOCAL_C_INCLUDES := $(BUSYBOX_INC)
LOCAL_CFLAGS := $(BUSYBOX_CFLAGS)

LOCAL_SHARED_LIBRARIES := liblog #libz #libclearsilver
LOCAL_STATIC_LIBRARIES := libclearsilverregex

LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
LOCAL_FORCE_STATIC_EXECUTABLE := true

$(LOCAL_MODULE_PATH)/busybox: BUSYBOX_BINARY := $(LOCAL_MODULE)
$(LOCAL_MODULE_PATH)/busybox: $(LOCAL_INSTALLED_MODULE)
	@echo "Symlink: $@ -> $(BUSYBOX_BINARY)"; ln -sf $(BUSYBOX_BINARY) $@

ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE_PATH)/busybox

LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)
endif

#include $(CLEAR_VARS)

BUSYBOX_INC :=
BUSYBOX_SRC :=
BUSYBOX_OBJS :=
BUSYBOX_CFLAGS :=

KERNELRELEASE :=
KBUILD_SRC :=

CPPFLAGS :=
OBJCOPY :=
OBJDUMP :=
LDFLAGS :=
LDLIBS :=
CFLAGS :=
AFLAGS :=
STRIP :=
#CXX :=
CPP :=
AR :=
AS :=
NM :=
LD :=
CC :=
# XXX:

# vim:sts=4:ts=8:
