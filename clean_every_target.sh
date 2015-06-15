#!/bin/sh
# Change the folder to YOUR sparrow3d folder!
cd ../sparrow3d
FILES=./target-files/*
echo "Cleaaning for all targets..."
for f in $FILES
do
	cd ../hase
	TARGET=`echo "$f" | cut -d/ -f3 | cut -d. -f1`
	make clean TARGET=$TARGET > /dev/null
	if [ $? -ne 0 ]; then
		echo "Error cleaning for \033[1;31m$TARGET\033[0m!"
	else
		echo "Everything cleaned for \033[1;32m$TARGET\033[0m!"
	fi
done
echo "Cleaning for default..."
make clean > /dev/null
