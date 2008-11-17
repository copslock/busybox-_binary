#!/bin/sh

#export TSLIB_CONFFILE=/etc/ts.conf;
#export TSLIB_CALIBFILE=/etc/pointercal;
#export TSLIB_TSDEVICE=/dev/input/event1;

export PATH=/bin:/sbin:/usr/bin:/usr/sbin
export TZ=cst-8; export HOME=/tmp; HN=Beagle; cd $HOME;

[ -f /proc/mounts ] && echo "shell initialized" && exit 0;
#[ -f /etc/issue ] && cat /etc/issue /etc/version;

#mount -a;	# XXX:

mount -t proc  none /proc -o noexec,nosuid,nodev;
mount -t sysfs none /sys  -o noexec,nosuid,nodev;

#[ -d /proc/bus/usb ]	 && mount -t usbfs usbfs /proc/bus/usb;
[ -d /sys/kernel/debug ] && mount -t debugfs debugfs /sys/kernel/debug;

#[ -d /sys/kernel/security ]     && \
#	mount -t securityfs securityfs /sys/kernel/security;
#[ -d /proc/sys/fs/binfmt_misc ] && \
#	mount -t binfmt_misc binfmt_misc /proc/sys/fs/binfmt_misc;
#grep -qs nfsd /proc/filesystems && mount -t nfsd nfsd /proc/fs/nfsd;

if [ -e /bin/mdev ] && [ -e /proc/sys/kernel/hotplug ]; then
    mount -t ramfs mdev /dev -o exec,nosuid,mode=0755 #,size=32K;

    mkdir -p /dev/shm /dev/pts /dev/input;
    mkdir -p /mnt/disk /mnt/media /mnt/config     /var/cache;
    mkdir -p /var/log /var/run /var/lock /var/lib /var/spool;

    echo /bin/mdev > /proc/sys/kernel/hotplug; /bin/mdev -s;
fi

grep -qs devpts /proc/filesystems && \
	mount -t devpts none /dev/pts -o noexec,nosuid,mode=0620;

#echo 9 > /proc/sys/kernel/printk;	# XXX:

#echo 80   > /proc/sys/vm/pagecache_ratio;
#echo 4096 > /proc/sys/vm/min_free_kbytes;
#echo 300  > /proc/sys/vm/vfs_cache_pressure;

#mkfs.vfat -F 32 -s 16;
#blockdev -v --setra 2048 /dev/mmcblk0p1;

#sleep 1 && \
#mount /dev/mmcblk0p1 /mnt/disk -o flush,noatime,utf8
#	context=system_u:object_r:removable_t;	# XXX:

#modprobe g_file_storage file=/dev/mmcblk0 stall=0 removable;

ifconfig lo 127.0.0.1 && hostname $HN; #hostname -F /etc/hostname;
echo "nameserver 202.38.64.1" > /tmp/resolv.conf;
#route add -net 127.0.0.0 netmask 255.0.0.0 lo;
#modprobe g_ether; NIF=usb0;	#modpeobe g_serial;

#ifconfig can0 up;	# XXX:

#if [ -e /proc/sys/net/ipv4/conf/$IF ]; then
#    IP=192.168.2.248;	GW=192.168.2.254;
#    PC=192.168.2.240;	NT=210.72.145.44;
#
#    ifconfig $IF $IP up;   MP=/mnt;
#    ping -c 1 $GW && route add -net default gw $GW;
#    echo "nameserver 202.102.192.68" >> /tmp/resolv.conf;
#
#    if ping -c 1 $PC > /dev/null 2>&1; then
#	nohup rdate -p -s $PC & #rdate -p -s $PC && date;
#	mount $PC:/tmp $MP -o nolock &	# XXX:
#    fi
#    #synergyc -n beagle $SV;	ntpdate $NT &
#    ping -c 1 $NT && ntpclient -s -c 0 -i 1020 -h $NT &
#
#    if [ -d /etc/cgi-bin ]; then
#	MP=/tmp;  mkdir $MP/cgi-bin;
#	ln -s ../../etc/cgi-bin/*.cgi $MP/cgi-bin/;
#	ln -s ../etc/cgi-bin/*.htm $MP/; fi
#
#   modprobe netconsole netconsole=@/,@192.168.2.140/;
#fi

#echo $HN; #hwclock -u -w;

#export PS1="[$HN: \\w> "
#[ -f /etc/motd ] && cat /etc/motd;

