#!/bin/sh
export PATH=":${PATH:-"/usr/bin:/bin:/usr/local/bin"}"
export LD_LIBRARY_PATH=":${LD_LIBRARY_PATH:-"/usr/lib:/lib"}"
export HOME="/mnt/utmp/hase" XDG_CONFIG_HOME="/mnt/utmp/hase"

if [ -d /mnt/utmp/hase/share ];then
	export XDG_DATA_DIRS=/mnt/utmp/hase/share:$XDG_DATA_DIRS:/usr/share
fi
export SDL_AUDIODRIVER="alsa"

./hase $*
