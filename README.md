# pivot

A fast implementation of the pivot algorithm for sampling the
[self-avoiding walk](https://en.wikipedia.org/wiki/Self-avoiding_walk).

## Description

The self-avoiding walk (SAW) is a model of linear polymers given by uniformly sampling
from the set of all possible paths on a lattice that do not intersect themselves.
In statistical physics, the main questions about this model revolve around its asymptotic
statistical properties as the number of steps in the path increases. These problems have
been studied using techniques from theoretical physics and rigorous mathematics as well
as computational methods, which are the focus of this project.

The *pivot algorithm* is a [Monte Carlo](https://en.wikipedia.org/wiki/Markov_chain_Monte_Carlo) approach
to simulating the SAW. The algorithm consists of a sequence of "pivot moves": random
rigid transformations applied at random points of a walk, thereby pivoting one end about the other.
In a [2010 paper](https://arxiv.org/abs/1005.1444), [Nathan Clisby](https://clisby.net) introduced the
*saw-tree* data structure, enabling a massive performance improvement to this algorithm.

This repository provides an implementation of the saw-tree pivot algorithm.

## Download

[Releases](https://github.com/bencwallace/pivot/releases) are available for Linux (x64) and macOS (arm64).

## Build

CMake and a suitable C/C++ compiler toolchain are required.

**Dependencies**

The following third-party dependencies are used:

* [boost](https://www.boost.org/doc/libs/1_85_0/more/getting_started/unix-variants.html) (headers only)
* [libgraphviz](https://gitlab.com/graphviz/graphviz)
  * Only needed at build time (optional at runtime)
* [CLI11](https://github.com/CLIUtils/CLI11)
  * Included via CMake

Only the first two requirements must be available prior to building. On Debian/Ubuntu,
this can be accomplished as follows:

```bash
sudo apt-get update && sudo apt-get install libboost-all-dev libgraphviz-dev
```

**Build**

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pivot_exec -j
```

**Maximum number of dimensions**

The maximum number of dimensions supported is a compile-time constant. It can be set by providing the `DIMS_UB` option to CMake,
which sets the exclusive upper bound on the dimensions supported. For instance, to support all dimensions up to and including 10,
one would set `DIMS_UB` to 11 as follows:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DDIMS_UB=11
cmake --build build --target pivot_exec -j
```

At the time of writing, the default value of `DIMS_UB` is 6. The most up-to-date default can be found by looking at
[CMakeLists.txt](CMakeLists.txt) or [defines.h](src/include/defines.h).

## Usage

For usage instructions, run the following command:

```
./build/pivot --help
```

## Examples

**Plotting a walk**

Attempt $10^5$ pivots on a $10^6$ step walk:

```
mkdir out
./build/pivot --steps 1000000 --iters 100000 --out out
```

Plot the output:

```
python plot.py out/walk.csv
```

**Estimating critical exponents**

The expected squared end-to-end distance $\langle |X(N)|^2 \rangle$ of an $N$-step self-avoiding walk
is believed to obey a power law of the form $C N^{2\nu}$. The exponent $\nu$ is an
example of a [critical exponent](https://en.wikipedia.org/wiki/Critical_exponent) and can be estimated
from samples gathered by running the pivot
algorithm over varying values of $N$.

In the example below, we increase the number of samples with the walk length as the pivot algorithm
needs a longer warm-up period to attain the equilibrium distribution for longer walks.

```bash
mkdir data
for i in $(seq 0 10)
do
  steps=$((1000 * 2 ** i))
  ./build/pivot --success -d 2 -s ${steps} -i $((2 * steps)) --out data
  mv data/endpoints.csv data/${steps}.csv
done
```

The resulting data can be analyzed using the tools of your choice. For instance, we can use Python
with numpy and scipy. Note that we ignore the warm-up samples for the purpose of this estimate.

```python
import csv
import numpy as np
from scipy.optimize import curve_fit

# parse data generated by the pivot algorithm
num_steps = [1000 * 2 ** i for i in range(11)]
data = {}
for n in num_steps:
  with open(f"data/{n}.csv") as f:
    reader = csv.reader(f)
    entries = [[int(x), int(y)] for x, y in list(reader)]
    data[n] = np.array(entries[(len(entries) // 2):])  # omit warm-up entries

# compute mean squared distance
squared_dists = {key: np.linalg.norm(val, ord=2, axis=1) ** 2 for key, val in data.items()}
mean_sq_dists = {key: np.mean(val) for key, val in squared_dists.items()}

# fit curve with scipy
def f(n, nu, C):
  return C * n ** (2 * nu)
params, _ = curve_fit(f, list(mean_sq_dists.keys()), list(mean_sq_dists.values()))
print(f"nu estimate: {params[0]}")
```

The output should be close to 3/4, the predicted value for $\nu$ in 2 dimensions.

## Limitations and future work

I hope to make the following changes in the future:

* Use Clisby's `fast_pivot` method
* Improve initialization methods (e.g. Clisby's `pseudo_dimerize` method)
* Support multithreaded pivot proposals

Some other interesting directions to explore would be the following:

* Add Python bindings
* Support non-cubic lattices
* Allow long-range step distributions
* Allow soft-core interactions (weak SAW / Domb-Joyce model)
