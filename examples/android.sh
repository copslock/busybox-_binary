#!/system/bin/sh -x
 ################################################################
 # $ID: android.sh     二, 14  6月 2011 18:47:49 +0800  mhfan $ #
 #                                                              #
 # Description:                                                 #
 #                                                              #
 # Maintainer:  范美辉(MeiHui FAN)  <mhfan@ustc.edu>            #
 #                                                              #
 # CopyLeft (c)  2011  M.H.Fan                                  #
 #   All rights reserved.                                       #
 #                                                              #
 # This file is free software;                                  #
 #   you are free to modify and/or redistribute it  	        #
 #   under the terms of the GNU General Public Licence (GPL).   #
 #                                                              #
 # Last modified: 二, 14  6月 2011 18:48:39 +0800      by mhfan #
 ################################################################

export PATH=/sbin:/system/sbin:/system/bin:$PATH
export LD_LIBRARY_PATH=/system/lib
export ANDROID_ASSETS=/system/app
export ANDROID_ROOT=/system
export ANDROID_DATA=/data

export EXTERNAL_STORAGE=/sdcard
export DRM_CONTENT=/data/drm/content

/system/bin/app_process -Xzygote /system/bin --zygote &
/system/bin/dbus-daemon --system &
runtime &

/system/bin/sh

 # vim:sts=4:ts=8:
