# pivot

Fast implementation of the pivot algorithm, based on Nathan Clisby's [2010 paper](https://arxiv.org/abs/1005.1444).

## Build

CMake and a suitable C/C++ compiler toolchain are required.

**Dependencies**

The following third-party dependencies are needed:

* [boost](https://www.boost.org/doc/libs/1_85_0/more/getting_started/unix-variants.html)
  * boost-unordered
  * boost-program-options
* [libgraphviz](https://gitlab.com/graphviz/graphviz)

For plotting, Python (with matplotlib) is required.

**Build**

```
cmake -S . -B build
cmake --build build
```

## Usage

For usage instructions, run the following command:

```
./build/pivot --help
```

## Examples

Attempt 100000 pivots on a 1000000 step walk:

```
./build/pivot --steps 1000000 --iters 100000
```

Plot the output:

```
python plot.py walk.csv
```
