#!/bin/sh

MP=/mnt/disk;

#GW=192.168.1.1;
#IP=192.168.1.53;
#SV=192.168.1.54;

IP=192.168.2.53;
GW=192.168.2.54;
SV=$GW;

SP=/home/mhfan/devel/beagle/rootnfs;

#modprobe twl4030_usb;		# XXX: g_cdc
#NC=/proc/sys/net/ipv4/conf;	#modprobe jz4740_udc use_dma=0;	# XXX:
(IF=usb0; MO=g_ether; modprobe $MO && sleep 2 && \
	(ifconfig $IF $IP up || (rmmod $MO; false))) || \
(IF=eth0; MO=dm9000 ; modprobe $MO && sleep 2 && \
	(ifconfig $IF $IP up || (rmmod $MO; false))) || return; # XXX: exit;
ping -c 1 $GW; route add default gw $GW; ping -c 1 $SV;

#x11vnc -q -bg -forever -avahi -ncache 10
#synergyc -n beagle $SV;	ntpdate 210.72.145.44 &		# XXX:
while mount -o nolock $SV:$SP $MP || (sleep 2; false); do   #,proto=tcp

    cd $MP; if [ $$ = 1 ]; then exec chroot . /sbin/init;
    elif echo $@ | grep -q reinit; then kill -3 1; else		# XXX:

	mount --rbind /sys sys;	mount --rbind /proc proc;
	mount --rbind /dev dev; mount --bind /dev/pts dev/pts;
	mount --rbind /dev/shm dev/shm;

	# XXX: other things to do here
	[ -x bin/bash ] && chroot . /bin/bash || chroot . #/bin/sh

	break;
    fi
done

