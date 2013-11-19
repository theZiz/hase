#!/bin/sh
export PATH=":${PATH:-"/usr/bin:/bin:/usr/local/bin"}"
export LD_LIBRARY_PATH=":${LD_LIBRARY_PATH:-"/usr/lib:/lib"}"
export HOME="/mnt/utmp/puzzletube" XDG_CONFIG_HOME="/mnt/utmp/puzzletube"

if [ -d /mnt/utmp/puzzletube/share ];then
	export XDG_DATA_DIRS=/mnt/utmp/puzzletube/share:$XDG_DATA_DIRS:/usr/share
fi
export SDL_AUDIODRIVER="alsa"

cat /proc/pandora/nub0/mode > /tmp/nub0mode_before_puzzletube
cat /proc/pandora/nub1/mode > /tmp/nub1mode_before_puzzletube
echo absolute > /proc/pandora/nub0/mode
echo absolute > /proc/pandora/nub1/mode

LD_LIBRARY_PATH=. ./puzzletube $*

cat /tmp/nub0mode_before_puzzletube > /proc/pandora/nub0/mode
cat /tmp/nub1mode_before_puzzletube > /proc/pandora/nub1/mode
 
rm /tmp/nub0mode_before_puzzletube /tmp/nub1mode_before_puzzletube
