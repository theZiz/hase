#!/bin/sh
# Change the folder to YOUR sparrow3d folder!
cd ../sparrow3d
FILES=./target-files/*
echo "Compiling for all targets..."
for f in $FILES
do
	cd ../hase
	TARGET=`echo "$f" | cut -d/ -f3 | cut -d. -f1`
	make oclean > /dev/null
	make TARGET=$TARGET > /dev/null
	if [ $? -ne 0 ]; then
		echo "Error compiling for \033[1;31m$TARGET\033[0m!"
	else
		echo "Everything fine with \033[1;32m$TARGET\033[0m!"
	fi
done
echo "Compiling for default..."
make clean > /dev/null
make > /dev/null
