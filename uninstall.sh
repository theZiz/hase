#!/bin/sh
if [ $# -gt 0 ]
then
	rm -f "$1/bin/hase"
	rm -rf "$1/share/hase"
	rm -f "$1/share/pixmaps/hase.png"
	rm -f "$1/share/applications/hase.desktop"
else
	echo "Usage: (sudo) ./uninstall.sh <install directory>"
	echo "e.g:"
	echo "\t sudo ./uninstall.sh /usr/local"
fi
