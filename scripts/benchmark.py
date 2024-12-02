import argparse
import matplotlib.pyplot as plt
import os
import time
from pathlib import Path

WARM_UP_FACTOR = 20
BENCH_ITERS = 1_000_000
STEPS = [1000, 10_000, 100_000, 1_000_000]

DEFAULT_PIVOT_PATH = Path(__file__).parent.parent / "build" / "pivot"
pivot_path = os.getenv("PIVOT_PATH", DEFAULT_PIVOT_PATH)


def main(dim):
    print(f"Running benchmark for {dim} dimensions")
    out_paths = {}
    times = {}
    for steps in STEPS:
        print(f"Running pivot with {steps} steps")

        out_dir = Path(__file__).parent / "benchmark" / f"dim_{dim}" / f"steps_{steps}"
        out_dir.mkdir(parents=True, exist_ok=True)
        out_paths[steps] = out_dir

        warm_up_iters = WARM_UP_FACTOR * steps
        print(f"Warming up for {warm_up_iters} iterations")
        os.system(f"{pivot_path} -d {dim} -s {steps} -i {warm_up_iters} --out {out_dir}")

        print(f"Running benchmark for {BENCH_ITERS} iterations")
        start = time.time()
        os.system(f"{pivot_path} -d {dim} -s {steps} -i {BENCH_ITERS} --in {out_dir}/walk.csv")
        stop = time.time()
        times[steps] = 1_000_000 * (stop - start) / BENCH_ITERS

    plt.loglog(STEPS, [times[steps] for steps in STEPS], marker="o")
    plt.title(f"Dimension {dim}")
    plt.xlabel("Number of steps")
    plt.ylabel("Microseconds per pivot attempt")
    plt.savefig(Path(__file__).parent / "benchmark" / f"dim_{dim}" / "times.png")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dim', type=int, default=2)

    args = parser.parse_args()
    main(args.dim)
