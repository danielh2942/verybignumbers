# Very Big Numbers
this is going to cover very big numbers and test them for efficiency and stuff.
This is for my final year project for BCT in University of Galway

## Requirements
- [Catch2](https://github.com/catchorg/Catch2) for unit testing. 
- CMake
- Ninja
- A C++20 compatible compiler.

## Setup
As this is packaged with CPM, it will automatically
handle dependencies (outside of stuff like ninja and cmake)
so you can run the following:
```bash
$ mkdir build
$ cd build
$ cmake -G "Ninja" ../
```

## TODO
- [X] Basic composite number type
- [X] --Trivially Human-Readable number type-- (Bad Idea (VERY VERY slow) but It's done)
    - [] Decimal place support
- [X] Negative Number support
- [] Non-statically sized type
- [X] Statically sized (But templated) numerical type
- [] Floating Point type
- [] Investigate whether or not AVX is practical
