#!/bin/bash
PROGRAM="hase"
VERSION="1.6.2.4"
DEST=./build/*
ZIP_CALL="7z a -t7z -m0=LZMA -mmt=on -mx=9 -md=256m -mfb=512 -ms=on upload.7z"
RM_CALL="rm upload.7z"
TIME=`date -u +"%d.%m.%Y %R"`

eval "$RM_CALL"

cat header.htm > index.htm
echo "<p>Updated at the $TIME.</p>" >> index.htm
echo "<ul>" >> index.htm
echo "<?php" > symlink.php
#echo "\$zip = new ZipArchive;" >> symlink.php
#echo "\$res = \$zip->open('upload.zip');" >> symlink.php
#echo "if (\$res === TRUE) {" >> symlink.php
#echo "  \$zip->extractTo('.');" >> symlink.php
#echo "  \$zip->close();" >> symlink.php
#echo "  unlink('upload.zip');" >> symlink.php
#echo "}" >> symlink.php
echo "system('7z e -y upload.7z');" >> symlink.php
echo "unlink('upload.7z');" >> symlink.php
for f in $DEST
do
	if [ -e "$f/$PROGRAM/$PROGRAM" ] || [ -e "$f/$PROGRAM/$PROGRAM.exe" ]; then
		NAME=`echo "$f" | cut -d/ -f3 | cut -d. -f1`
		echo "$NAME:"
		echo "--> Copy temporary folders"
		cp -r data "$f/$PROGRAM"
		cp -r textures "$f/$PROGRAM"
		cp -r sprites "$f/$PROGRAM"
		cp -r sounds "$f/$PROGRAM"
		cp hase_readme.txt "$f/$PROGRAM"
		cd $f
		echo "--> Create archive"
		if [ $NAME = "pandora" ]; then
			cd $PROGRAM
			../make_package.sh
			cd ..
			echo "<li><a href=$PROGRAM.pnd>$NAME</a></li>" >> ../../index.htm
			ZIP_CALL+=" $PROGRAM.pnd"
		else
			if [ $NAME = "i386" ] || [ $NAME = "amd64" ]; then
				tar cfvz "$PROGRAM-$NAME-$VERSION.tar.gz" * > /dev/null
				mv "$PROGRAM-$NAME-$VERSION.tar.gz" ../..
				echo "<li><a href=$PROGRAM-$NAME-$VERSION.tar.gz>$NAME</a></li>" >> ../../index.htm
				echo "unlink('$PROGRAM-$NAME.tar.gz');" >> ../../symlink.php
				echo "symlink('$PROGRAM-$NAME-$VERSION.tar.gz', '$PROGRAM-$NAME.tar.gz');" >> ../../symlink.php
				ZIP_CALL+=" $PROGRAM-$NAME-$VERSION.tar.gz"
			else
				#no Pandora, no PC. So we have a low resolution device!
				if [ $NAME != "win32" ]; then
					cp ../../small_resolution_data/* "$PROGRAM/data"
				fi
				if [ $NAME = "gcw" ]; then
					cp ../../small_resolution_censorship/* "$PROGRAM/data"
					mksquashfs * "$PROGRAM-$VERSION.opk" -all-root -noappend -no-exports -no-xattrs
					mv "$PROGRAM-$VERSION.opk" ../..
					echo "<li><a href=$PROGRAM.opk type=\"application/x-opk+squashfs\">$NAME</a></li>" >> ../../index.htm
					echo "unlink('$PROGRAM.opk');" >> ../../symlink.php
					echo "symlink('$PROGRAM-$VERSION.opk', '$PROGRAM.opk');" >> ../../symlink.php
					ZIP_CALL+=" $PROGRAM-$VERSION.opk"
				else
					zip -9 -r "$PROGRAM-$NAME-$VERSION.zip" * > /dev/null
					mv "$PROGRAM-$NAME-$VERSION.zip" ../..
					echo "<li><a href=$PROGRAM-$NAME-$VERSION.zip>$NAME</a></li>" >> ../../index.htm
					echo "unlink('$PROGRAM-$NAME.zip');" >> ../../symlink.php
					echo "symlink('$PROGRAM-$NAME-$VERSION.zip', '$PROGRAM-$NAME.zip');" >> ../../symlink.php
					ZIP_CALL+=" $PROGRAM-$NAME-$VERSION.zip"
				fi
			fi
		fi
		echo "--> Remove temporary folders"
		rm -r $PROGRAM/data
		rm -r $PROGRAM/textures
		rm -r $PROGRAM/sprites
		rm -r $PROGRAM/sounds
		rm $PROGRAM/hase_readme.txt
		cd ..
		cd ..
	fi
done
echo "?>" >> symlink.php
cat footer.htm >> index.htm
eval "$ZIP_CALL"
