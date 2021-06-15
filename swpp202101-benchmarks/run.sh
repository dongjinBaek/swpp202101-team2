#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run.sh <clang dir>"
  echo "ex)  ./run.sh ~/llvm-12.0-releaseassert/bin"
  exit 1
fi

for pathname in *; do
    if [ -d $pathname ]; then
        echo $pathname
        ./c-to-ll.sh "$pathname/src/$pathname.c" $1
        cd "$pathname/src"
        sf-compiler "$pathname.ll" "$pathname.s" > "$pathname.opt.ll"
        cd "../test"
        for inputname in input*.txt; do
            echo "    $inputname"
            sf-interpreter "../src/$pathname.s" < $inputname > output.txt
            mv "sf-interpreter.log" "$inputname.log"
            mv "sf-interpreter-cost.log" "$inputname-cost.log"
            num=$(echo $inputname | cut -c6-)
            num=${num%.txt}
            diff output.txt "output$num.txt" >> "$inputname.log"
        done
        rm output.txt
        cd ../../
    fi
done
