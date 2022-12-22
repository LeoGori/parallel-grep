# Parallel grep
This project consists in a simplified implementation of a **string matching algorithm**, with the introduction of parallel computation through the use of [**Microsoft MPI**](https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi) APIs.

## Code Description
The main files that build up the project are:
- [**_grep.h_**](https://github.com/LeoGori/parallel-grep/blob/main/grep.h): class that defines the **grep** namespaces, which includes data types and functiions that are useful for the implementation of the algorithm.
- [**_grep-main.h_**](https://github.com/LeoGori/parallel-grep/blob/main/grep-main.cpp): the entry point fo the program, which implements the string matching algorithm, with the management of multiple processes.

## Language and APIs
The code is entirely written in C++ programming language, with the use of the following libraries and APIs (omitting the standard ones):
- [**_fstream_**](https://cplusplus.com/reference/fstream/fstream/): used for parsing the text file
- [**_cstring_**](https://cplusplus.com/reference/cstring/): used for copying the content of vectors of strings into an unique array of chars, that is necessary as a consistent input for MPI functions
- [**_mpi.h_**](https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi): which introduces parallel computation in the algorithm, through a message passing mechanism between processes.

## Performance analysis
Check the analysis of the performances of the implementation [here](https://colab.research.google.com/drive/19F67NHWAFvQK0EPysXL6RJyh3ffKjd56?usp=sharing)

## How to run the code (Windows)

1. Install [MinGw64](https://winlibs.com/)
2. Install [CMake](https://cmake.org/download/) version &ge; 3.23 
3. Create folder for building project
```
  mkdir build
  cd build
```
4.Generate the makefiles
```
  cmake -G “MinGW Makefiles” ..
```
5. build the project
```
  cmake --build .
```
6. run the program
```
  mpiexec -np $num_proc$ .\parallel_grep.exe happy ..\resources\input_file.txt
```
where \$ num_proc \$ needs to be replaced by a positive integer representing the number of processes that will execute the code
