#
# Regular cron jobs for the busybox package
#
0 4	* * *	root	[ -x /usr/bin/busybox_maintenance ] && /usr/bin/busybox_maintenance
