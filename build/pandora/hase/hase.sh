#!/bin/sh
export PATH=":${PATH:-"/usr/bin:/bin:/usr/local/bin"}"
export LD_LIBRARY_PATH=":${LD_LIBRARY_PATH:-"/usr/lib:/lib"}"
export HOME="/mnt/utmp/hase" XDG_CONFIG_HOME="/mnt/utmp/hase"

if [ -d /mnt/utmp/hase/share ];then
	export XDG_DATA_DIRS=/mnt/utmp/hase/share:$XDG_DATA_DIRS:/usr/share
fi
export SDL_AUDIODRIVER="alsa"

cat /proc/pandora/nub0/mode > /tmp/nub0mode_before_hase
cat /proc/pandora/nub1/mode > /tmp/nub1mode_before_hase
echo absolute > /proc/pandora/nub0/mode
echo absolute > /proc/pandora/nub1/mode

LD_LIBRARY_PATH=. ./hase $*

cat /tmp/nub0mode_before_hase > /proc/pandora/nub0/mode
cat /tmp/nub1mode_before_hase > /proc/pandora/nub1/mode
 
rm /tmp/nub0mode_before_hase /tmp/nub1mode_before_hase
