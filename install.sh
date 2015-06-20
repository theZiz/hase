#!/bin/sh
if [ $# -gt 1 ]
then
	make clean
	make PARAMETER="-DDATA_FOLDER=\"\\\"$2/hase\\\"\"" NO_DEBUG=1
	cp hase "$1/hase"
	if [ ! -d "$2/hase" ]; then mkdir "$2/hase";fi
	cp -r data "$2/hase"
	cp -r textures "$2/hase"
	cp -r sprites "$2/hase"
	cp -r sounds "$2/hase"
	cp hase_readme.txt "$2/hase"
	cp hase.png "$2/pixmaps"
	cp hase.desktop "$2/applications"
	make clean
else
	echo "Usage: (sudo) ./install.sh <bin directory> <share directory>"
	echo "e.g:"
	echo "\t sudo ./install.sh /usr/local/bin /usr/local/share"
fi
