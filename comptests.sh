#!/bin/bash

YEAR=$(pwd | grep -o '20..-.')
IN=tests3
# OUT=tests/out

EXE=./trabalho

for subfolder in `ls $IN/`; do
    for infile in `ls $IN/$subfolder/*.c`; do
        base=$(basename $infile)
        # outfile=${base/.c/.}
        echo $infile dots/${base/.c/.dot} asms/${base/.c/.asm}
        $EXE <$infile 2>dots/${base/.c/.dot} 1>asms/${base/.c/.asm}
    done
done
