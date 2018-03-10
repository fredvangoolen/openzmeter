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
        *.css|*.js) echo -n "Append and minimify " $SHORTFILENAME
                    yui-compressor $FILE > $TMPFILE
                    printf "%s\x00%d\x00" $SHORTFILENAME `stat --printf="%s" $TMPFILE` >> $TMPOUT
                    INSIZE=`stat --printf="%s" $FILE`
                    OUTSIZE=`stat --printf="%s" $TMPFILE`
                    PERCENT=$(( 100 - (100 * $OUTSIZE) / $INSIZE))
                    printf  " from %d to %d (%d%% saved)\n" $INSIZE $OUTSIZE $PERCENT
                    cat $TMPFILE >> $TMPOUT
                    rm $TMPFILE 
        ;;
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