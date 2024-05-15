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

**Plotting a walk**

Attempt 100000 pivots on a 1000000 step walk:

```
./build/pivot --steps 1000000 --iters 100000
```

Plot the output:

```
python plot.py walk.csv
```

**Estimating critical exponents**

The expected squared end-to-end distance of a self-avoiding walk is believed to obey a power law
governed by the critical exponent $\nu$. This exponent can be estimated from samples gathered
by running the pivot algorithm over varying lengths (numbers of steps).

In the example below, we increase the number of samples with the walk length as the pivot algorithm
needs a longer warm-up period to attain the equilibrium distribution.

```bash
mkdir data
for i in $(seq 0 10)
do
  steps=$((1000 * 2 ** i))
  ./build/pivot --success 1 --steps ${steps} --iters $((2 * steps)) --out data
  mv data/endpoints.csv data/${steps}.csv
done
```

The resulting data can be analyzed using the tools of your choice. For instance, we can use Python
with numpy and scipy. Note that we ignore the warm-up samples for the purpose of this estimate.

```python
import csv
import numpy as np
from scipy.optimize import curve_fit

num_steps = [1000 * 2 ** i for i in range(11)]
data = {}
for n in num_steps:
  with open(f"data/{n}.csv") as f:
    reader = csv.reader(f)
    entries = [[int(x), int(y)] for x, y in list(reader)]
    data[n] = np.array(entries[(len(entries) // 2):])

squared_dists = {key: np.linalg.norm(val, ord=2, axis=1) ** 2 for key, val in data.items()}
mean_sq_dists = {key: np.mean(val) for key, val in squared_dists.items()}

def f(n, nu, C):
  return C * n ** (2 * nu)

params, _ = curve_fit(f, list(mean_sq_dists.keys()), list(mean_sq_dists.values()))
print(f"nu estimate: {params[0]}")
```

The output should be close to 0.75, the predicted value for $\nu$ in 2 dimensions.

## Limitations and future work

I hope to make the following changes in the future:

* Lift restriction to $d = 2$ dimensions
* Use Clisby's `fast_pivot` method
* Improve initialization methods (e.g. Clisby's `pseudo_dimerize` method)
* Add Python bindings
* Support multithreaded pivot proposals
* Support non-cubic lattices
* Allow long-range step distributions
* Allow soft-core interactions (weak SAW / Domb-Joyce model)
