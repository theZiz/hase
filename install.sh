#!/bin/sh
if [ $# -gt 0 ]
then
	make clean
	make PARAMETER="-DDATA_FOLDER=\"\\\"$1/share/hase\\\"\"" NO_DEBUG=1
	cp hase "$1/bin"
	if [ ! -d "$1/share/hase" ]; then mkdir "$1/share/hase";fi
	cp -r data "$1/share/hase"
	cp -r textures "$1/share/hase"
	cp -r sprites "$1/share/hase"
	cp -r sounds "$1/share/hase"
	cp hase_readme.txt "$1/share/hase"
	cp hase.png "$1/share/pixmaps"
	cp hase.desktop "$1/share/applications"
	make clean
else
	echo "Usage: (sudo) ./install.sh <install directory>"
	echo "e.g:"
	echo "\t sudo ./install.sh /usr/local"
	echo "which will install hase to /usr/local/bin and /usr/local/share"
fi
