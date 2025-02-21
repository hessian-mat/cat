<p align="center"><img src="https://github.com/hessian-mat/cat/blob/main/logo.webp" alt="cat logo" width="200"></p>
<h1 align="center">cat: C Accessories Toolkit</h1>

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Generic implemetation of containers in C & more.

- [Build \& Install](#build--install)
  - [UNIX](#unix)
  - [Windows](#windows)
- [Features](#features)
- [Usage](#usage)
- [Related projects](#related-projects)

## Build & Install
Building the library requires CMake 3.10 or later. The installation includes a static library, 
a dynamic library and the header files.
### UNIX
To install the library on a UNIX environment, run:
```sh
git clone https://github.com/hessian-mat/cat.git
cd cat
mkdir build && cd build
cmake ..
make
sudo make install
```
To verify the installation, run in `build` directory:
```sh
make test
```
or execute test binaries directly to see the verbose output:
```sh
./test_array # run test for array
```
By default the installation path is `/usr/local`.

### Windows
Only MSVC is tested on Windows. To install the library on Windows, run(in git-bash):
```sh
git clone https://github.com/hessian-mat/cat.git
cd cat
mkdir build && cd build
cmake -G "Visual Studio 17 2022" .. # suppose VS is used as generator
cmake --build . --target install
```
Alternatively, you can open the `cat.sln` file with Visual Studio and build the project.
The default installation path is `$HOME\cat`.

## Features
The containers are implemented as generic types, allowing them to store elements of 
any type. During the initialization, the size of the element and the initial capacity 
of the container are required. Containers dynamically expand by reallocating memory 
when the container is full.

The containers store elements directly in a `void*` buffer and allocate memory for 
them. For simple types (e.g. int, float, char, etc.) and structs, the elements are 
stored directly in the buffer and freed automatically when the container is deinitialized. 
However, for some pointer types, the container actually stores the pointer values, so 
custom memory management of the pointed-to data is required. All the containers support 
custom memory allocation functions and free functions provided by the user to override 
the default malloc/free.

| Container | Type | Description |
|-----------|-------------|-------------|
| Array | `array_t` | A dynamic array that stores elements continuously |
| Deque | `deque_t` | A double-ended queue that stores elements in a circular buffer |
| HashMap | `hashmap_t` | An unordered map that stores key-value pairs |
| List | `list_t` | A doubly-linked list that stores elements separately |
| Priority Queue | `pqueue_t` | A priority queue implemented as a binary heap |
| String | `string_t` | A string type |

Most functions have return values as error codes. There are five error codes in total:
- `COMPLETE`: No error
- `ERR_INVALID_OPERATION`: The operation is invalid(e.g. pop from an empty container, remove a non-existent element)
- `ERR_MEMORY_ALLOCATION`: Memory allocation failed
- `ERR_CAPACITY_OVERFLOW`: The capacity of the container is overflow
- `ERR_INDEX_OUT_OF_RANGE`: The index is out of range

The API is simple and similar to C++ STL. For more detailed documentation, refer to
the source files.

## Usage
Here's a simple helloworld program:
```c
// main.c
#include <stdio.h>
#include "cat_array.h"

int main() {
    // Initialize an array of 5 char*
    array_t arr = array(char*, 5);

    // Push 10 "Hello, world!" literals to the array
    char* str = "Hello, world!";
    for (int i = 0; i < 10; i++) {
        array_push_back(arr, &str);
    }

    // Get the element at index 5
    char* r;
    array_get(arr, &r, 5);
    printf("%s\n", r);

    // Free all the memory associated with the array
    array_deinit(arr);
    return 0;
}
```
For the static library:
```sh
gcc -o helloworld main.c /usr/local/lib/libcat.a
```
For the dynamic library:
```sh
export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH
gcc -o helloworld main.c -lcat
```
Run the helloworld program and a `"Hello, world!"` will be printed! More examples can 
be found in the `test` directory.

## Related projects
- https://github.com/bkthomps/Containers: Another C++ STL-like container library.
