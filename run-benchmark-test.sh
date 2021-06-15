if [ "$#" -ne 1 ]; then
    echo "run.sh <clang dir>"
    echo "ex)  ./run.sh ~/llvm-12.0-releaseassert/bin"
    exit 1
fi

cd swpp202101-interpreter
./build.sh
cd ../swpp202101-benchmarks
./build-irgen.sh $1

for pathname in *; do
    if [ -d $pathname ]; then
        echo $pathname
        ./c-to-ll.sh "$pathname/src/$pathname.c" $1
        cd "$pathname/src"
        ../../../bin/sf-compiler "$pathname.ll" "$pathname.s" > "$pathname.opt.ll"
        cd "../test"
        for inputname in input*.txt; do
            ../../../bin/sf-interpreter "../src/$pathname.s" < $inputname > output.txt
            mv "sf-interpreter.log" "$inputname.log"
            mv "sf-interpreter-cost.log" "$inputname-cost.log"
            out=$(echo $inputname | cut -c6-)
            diff output.txt "output$out" >> "$inputname.log"
            error=$?
            if [ $error -eq 0 ]
            then
                echo "  $inputname passed test"
            elif [ $error -eq 1 ]
            then
                echo "  $inputname test error"
                exit 1
            fi
        done
        rm output.txt
        cd ../../
    fi
 done