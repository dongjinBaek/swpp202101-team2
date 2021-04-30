cmake_minimum_required(VERSION 3.10)
project(swpp_asm_interpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -flto")
# set(CMAKE_CXX_FLAGS "-g")

add_executable(
        sf-interpreter
        src/main.cpp
        src/opcode.h
        src/value.h
        src/value.cpp
        src/size.h
        src/size.cpp
        src/stmt.h
        src/stmt.cpp
        src/reg.h
        src/regfile.h
        src/regfile.cpp
        src/error.h
        src/error.cpp
        src/memory.h
        src/memory.cpp
        src/function.h
        src/function.cpp
        src/program.h
        src/program.cpp
        src/state.h
        src/state.cpp
        src/parser.h
        src/parser.cpp
        )
