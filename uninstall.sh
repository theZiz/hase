#!/bin/sh
if [ $# -gt 1 ]
then
	rm -f "$1/hase"
	rm -rf "$2/hase"
	rm -f "$2/pixmaps/hase.png"
	rm -f "$2/applications/hase.desktop"
else
	echo "Usage: (sudo) ./uninstall.sh <bin directory> <share directory>"
	echo "e.g:"
	echo "\t sudo ./uninstall.sh /usr/local/bin /usr/local/share"
fi
