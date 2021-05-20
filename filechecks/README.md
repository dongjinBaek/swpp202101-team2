# Writing Filecheck Tests

## 1. Make .ll file
- .ll files should go under filechecks directory
- Filecheck testfile for pass 'MyCustomPass' should be named `MyCustomPass_TestN.ll`

## 2. Modify main.cpp
- adding your pass should go inside if clause, calling `shouldUsePass` function
- `shouldUsePass` function will return true in 2 cases
    1. no -p argument was given to sf-compiler
    2. one of -p argument matches with function argument
- run-filechecks.sh will give `-p MyCustomPass` option when running MyCustomPass_TestN.ll
```cpp
    if (shouldUsePass("MyCustomPass")) {
        FPM.addPass(MyCustomPass());
    }
```