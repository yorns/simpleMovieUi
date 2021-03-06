#!/bin/sh
# .bashrc is sourced only for non-login shells. For console login shells 
# only .profile (or .bash_profile or .bash_login) is sourced.

cp /etc/udev/scripts/mount1.sh /etc/udev/scripts/mount.sh
/bin/echo "3" > /proc/sys/kernel/printk
if [ -f /usr/bin/mount_pre.sh ]
then
   /usr/bin/mount_pre.sh
fi
setfont /usr/share/consolefonts/Lat15-TerminusBold32x16.psf.gz
/usr/bin/ui /tmp
bash
 
