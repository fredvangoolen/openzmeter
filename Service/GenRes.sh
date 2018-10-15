#!/bin/bash
IFS=$'\n'
cd WEB
FILES=`find ./ ! -type d -print | sort`
BUILDDIR=../build/
TMPOUT=../build/ResourceFiles
TMPFILE=../build/MinifyResource.tmp
mkdir -p $BUILDDIR
rm -rf $TMPFILE $TMPOUT
LEN=`find ./ ! -type d -print | wc -l`
printf "%s\x00" $LEN > $TMPOUT
echo "Saving " $LEN " files"
for FILE in $FILES
do
    SHORTFILENAME=`echo $FILE | cut -c3-`
    case $FILE in
        *) echo "Append " $SHORTFILENAME
           printf "%s\x00%d\x00" $SHORTFILENAME `stat --printf="%s" $FILE` >> $TMPOUT
           cat $FILE >> $TMPOUT
    esac
done
echo "Pack finished, total size => " `stat --printf="%s" $TMPOUT`
cd ..
cd build
ld -r -b binary -o ResourceFiles.o ResourceFiles
rm ResourceFiles
objdump -t ResourceFiles.o
cd ..

