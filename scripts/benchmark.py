import argparse
import json
import matplotlib.pyplot as plt
import os
import subprocess
import time
from pathlib import Path

WARM_UP_FACTOR = 20
BENCH_ITERS = 1_000_000

DEFAULT_PIVOT_PATH = Path(__file__).parent.parent / "build" / "pivot"
pivot_path = os.getenv("PIVOT_PATH", DEFAULT_PIVOT_PATH)


def _steps(max_power):
    return [2 ** i - 1 for i in range(2, max_power + 1)]


def warmup(dim: int, max_power: int, seed: int | None = None):
    steps_list = _steps(max_power)
    print(f"Running warmup for dimension {dim}")
    for steps in steps_list:
        warm_up_iters = WARM_UP_FACTOR * steps
        print(f"Warming up with {steps} steps for {warm_up_iters} iterations")

        out_dir = Path(__file__).parent / "benchmark" / f"dim_{dim}" / f"warmup_{steps}"
        out_dir.mkdir(parents=True, exist_ok=True)

        cmd = f"{pivot_path} -d {dim} -s {steps} -i {warm_up_iters} --out {out_dir} "
        if seed is not None:
            cmd += f"--seed {seed}"
        os.system(cmd)
        print(f"Checkpoint saved to {out_dir}/walk.csv")


def benchmark(dim: int, slow: bool, max_power: int, naive: bool = False, seed: int | None = None):
    if naive:
        slow = True
    print(f"Running benchmark for dimension {dim}")
    times = {}
    steps_list = _steps(max_power)
    for steps in steps_list:
        print(f"Running benchmark with {steps} steps for {BENCH_ITERS} iterations")

        in_dir = Path(__file__).parent / "benchmark" / f"dim_{dim}" / f"warmup_{steps}/walk.csv"
        cmd = (
            f"{pivot_path} "
            f"-d {dim} "
            f"-s {steps} "
            f"-i {BENCH_ITERS} "
            f"--in {in_dir} "
            f"--{'slow' if slow else 'fast'} "
            f"{'--naive' if naive else ''} "
        )
        if seed is not None:
            cmd += f"--seed {seed}"
        proc = subprocess.Popen(cmd, shell=True, stderr=subprocess.PIPE)
        proc.stderr.readline()
        start = time.time()
        proc.wait()
        if proc.returncode != 0:
            print(cmd)
            print(proc.stderr.read().decode())
            raise RuntimeError(f"Benchmark failed with return code {proc.returncode}")
        stop = time.time()
        times[steps] = 1_000_000 * (stop - start) / BENCH_ITERS
        print(f"Time per pivot attempt: {times[steps]:.2f} µs")

    out_path = Path(__file__).parent / "benchmark" / f"dim_{dim}" / "times.json"
    with open(out_path, "w") as f:
        json.dump(times, f)
    print(f"Times saved to {out_path}")

def analyze(dim):
    with open(Path(__file__).parent / "benchmark" / f"dim_{dim}" / "times.json", "r") as f:
        times = json.load(f)
    times = {int(k): v for k, v in times.items()}

    steps_list = sorted(times.keys())
    plt.plot(steps_list, [times[steps] for steps in steps_list], marker="o")
    plt.title(f"Dimension {dim}")
    plt.xlabel("Number of steps")
    plt.ylabel("Microseconds per pivot attempt")
    plt.xscale("log", base=2)
    plt.savefig(Path(__file__).parent / "benchmark" / f"dim_{dim}" / "times.png")
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--dim", type=int, default=2)
    parser.add_argument("--max-power", type=int, default=19)
    subparsers = parser.add_subparsers(dest="command")

    warmup_parser = subparsers.add_parser("warmup")
    warmup_parser.add_argument("--seed", default=None)

    benchmark_parser = subparsers.add_parser("benchmark")
    benchmark_parser.add_argument("--slow", default=False, action="store_true")
    benchmark_parser.add_argument("--seed", default=None)
    benchmark_parser.add_argument("--naive", default=False, action="store_true")

    analyze_parser = subparsers.add_parser("analyze")

    args = parser.parse_args()
    if args.command == "warmup":
        warmup(args.dim, args.max_power, args.seed)
    elif args.command == "benchmark":
        benchmark(args.dim, args.slow, args.max_power, naive=args.naive, seed=args.seed)
    elif args.command == "analyze":
        analyze(args.dim)
