#!/bin/bash

YEAR=$(pwd | grep -o '20..-.')
IN=tests2
# OUT=tests/out

EXE=./trabalho

for subfolder in `ls $IN/`; do
    for infile in `ls $IN/$subfolder/*.c`; do
        base=$(basename $infile)
        # outfile=$OUT/${base/.ezl/.out}
        echo $infile
        $EXE < $infile
    done
done
