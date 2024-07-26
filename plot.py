import csv
import sys


def main(path, use_plotly=False):
    with open(path) as f:
        reader = csv.reader(f)
        data = [[int(y) for y in x] for x in reader]

    x, y = zip(*data)
    if use_plotly:
        import plotly.graph_objects as go
        fig = go.Figure(data=go.Scatter(x=x, y=y))
        fig.show()
    else:
        import matplotlib.pyplot as plt
        plt.plot(x, y)
        plt.show()


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else False)
