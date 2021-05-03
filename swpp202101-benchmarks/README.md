# Benchmarks for SWPP Team Project

This repository contains C programs, IR programs that are compiled from them, and input/outputs
for the team project of [SWPP](https://github.com/snu-sf-class/swpp202101).

Note that in the final competition TAs can add more test cases or programs.

### Authors

Sung-Hwan Lee: binary_tree, bubble_sort, collatz, gcd, prime, matmul1~4

Juneyoung Lee: bitcountN, scripts, rmq, all other benchmarks used for competition

### How to build .ll file from your own C program?

1. Build `irgen`.

```
./build-irgen.sh <llvm-12.0-releaseassert/bin>
```

2. Convert .c to .ll using c-to-ll.sh.
  
```
./c-to-ll.sh <.c file> <llvm-12.0-releaseassert/bin>"
```

## Improved version
### Build compiler
Go to compiler repository (`swpp202101-team2`) and build the compiler.
Register the compiler to PATH. Here is the example.
```
export PATH="/home/whnbaek/SW2021/swpp202101-team2/bin:$PATH"
```

### Build interpreter
Go to interpreter repository (`swpp202101-team2/swpp202101-interpreter`) and build the interpreter. Path of interpreter binary file is changed into `../bin` so you don't have to add this too.

### Run benchmarks
Finally, go to benchmark repository (`swpp202101-team2/swpp202101-benchmarks`) and do the below.
```
./build-irgen.sh <llvm-12.0-releaseassert/bin>
./run.sh <llvm-12.0-releaseassert/bin>
```
`irgen`, `c-to-ll.sh`, `sf-compiler` and `sf-interpreter` are executed inside `run.sh`. It generates LLVM IRs and assembly files inside `src` directory. Then it executes the assembly files with test cases and write logs inside `test` directory. The logging information is improved. You can check this out.
