#!/bin/sh
# .bashrc is sourced only for non-login shells. For console login shells 
# only .profile (or .bash_profile or .bash_login) is sourced.

setfont /home/root/Lat15-TerminusBold32x16.psf.gz
sleep 2
mount /dev/sda1 /media/usb2
if [ ! -e /media/usb2/database ] ; then
  /home/root/ui /media/usb2/database.json
else
  echo "No database available"
fi
bash
 
