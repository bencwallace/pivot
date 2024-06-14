import json
import os
import subprocess
from tempfile import TemporaryDirectory
from timeit import timeit

import matplotlib.pyplot as plt

def main():
    iters = int(1E6)
    results = {}
    with TemporaryDirectory() as tmpdir:
        for i in range(10, 20):
            steps = 2 ** i
            print(steps)
            # subprocess.run(f"{os.getcwd()}/build/pivot -d 2 -s {steps} -i {100 * steps} --out {tmpdir}", shell=True, check=True)
            results[steps] = timeit(
                stmt=f"subprocess.run('{os.getcwd()}/build/pivot -d 2 -s {steps} -i {iters}', shell=True)",
                setup="import subprocess",
                number=1
            )

    for steps, result in results.items():
        print(f"steps: {steps}, time: {result}")
    
    # plt.xscale("log")
    # plt.yscale("log")
    # plt.plot(results.keys(), results.values())
    # # plt.semilogy(results.keys(), results.values())
    # plt.show()

    with open("benchmark.json", "w") as f:
        json.dump(results, f)

if __name__ == "__main__":
    main()
