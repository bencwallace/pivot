import csv
import sys

import matplotlib.pyplot as plt


def main(path):
    with open(path) as f:
        reader = csv.reader(f)
        data = [[int(y) for y in x] for x in reader]

    plt.plot(*zip(*data))
    plt.show()


if __name__ == "__main__":
    main(sys.argv[1])
