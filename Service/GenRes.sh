#!/bin/bash
FILES=`find ./WEB ! -type d -print`
TMPOUT=build/Resources.bin
TMPFILE=build/MinifyResource.tmp
OBJECTFILE=build/Resources.o
TESTARCHFILE=build/Default/GNU-Linux/Main.o
rm $TMPFILE $TMPOUT $OBJECTFILE
LEN=`find ./WEB ! -type d -print | wc -l`
printf "%s\x00" $LEN > $TMPOUT
ELFARCH=`objdump -f $TESTARCHFILE | grep -o 'elf.*'`
OUTARCH=`objdump -f $TESTARCHFILE | head -3 | tail -1 | grep -o :.*: | cut -d : -f 2-2`
echo "Saving " $LEN " files in arch " $ELFARCH
for FILE in $FILES
do
    case $FILE in
        *.css|*.js) echo -n "Append and minimify " $FILE
                    yui-compressor $FILE > $TMPFILE
                    printf "%s\x00\%d\x00" $FILE `stat --printf="%s" $TMPFILE` >> $TMPOUT
                    INSIZE=`stat --printf="%s" $FILE`
                    OUTSIZE=`stat --printf="%s" $TMPFILE`
                    PERCENT=$(( 100 - (100 * $OUTSIZE) / $INSIZE))
                    printf  " from %d to %d (%d%% saved)\n" $INSIZE $OUTSIZE $PERCENT
                    cat $TMPFILE >> $TMPOUT
        ;;
        *) echo "Append " $FILE
           printf "%s\x00\%d\x00" $FILE `stat --printf="%s" $FILE` >> $TMPOUT
           cat $FILE >> $TMPOUT
    esac
done
echo "Pack finished, total size => " `stat --printf="%s" $TMPOUT`
objcopy --input binary --output $ELFARCH --binary-architecture $OUTARCH --rename-section .data=.rodata,load,alloc,readonly,data,contents $TMPOUT $OBJECTFILE
rm $TMPOUT
objdump -f $OBJECTFILE
