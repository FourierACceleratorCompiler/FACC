import argparse
import matplotlib.pyplot as plt
import matplotlib.ticker as plticker
import plot
import plot_ffta
import numpy as np
import csv

class Result:
    def __init__(self, name, compiled, matched, unmatched):
        self.name = name
        self.compiled = compiled
        self.matched = matched
        self.unmatched = unmatched

def load_result_from_row(rows):
    return Result(rows[0], float(rows[1]), float(rows[2]), float(rows[3]))

def plot_graph(results):
    names = []
    compiled = []
    matched = []
    unmatched = []

    for i in results:
        names.append(i.name)
        compiled.append(i.compiled)
        matched.append(i.matched)
        unmatched.append(i.unmatched)
    print("Plotting " + str(len(names)))

    plt.figure(figsize=(6, 3))
    plt.ylabel('Fraction of FFTs')
    xpos = np.arange(0, len(names))
    width = 0.25
    plt.ylim([0, 1])
    plt.bar(xpos, compiled, width, label='Compiled', color='blue', hatch='o')
    plt.bar(xpos + width, matched, width, label='Matched', color='red', hatch='-')
    plt.bar(xpos + 2 * width, unmatched, width, label='Unmatched', color='purple')
    plt.legend(loc="upper left")

    ax = plt.gca()
    yticks = plticker.FixedLocator([0.2, 0.4, 0.6, 0.8])
    xticks = plticker.FixedLocator(xpos[:-1] + 0.75)
    ax.xaxis.set_minor_locator(xticks)
    ax.yaxis.set_major_locator(yticks)
    ax.grid(which='minor', axis='x')
    ax.grid(which='major', axis='y')
    plt.xticks(xpos + width, names)

    plt.tight_layout()
    plt.savefig('fraction_supported.eps')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file')
    args = parser.parse_args()

    results = []

    with open(args.input_file) as csvfile:
        reader = csv.reader(csvfile)
        header = True
        for row in reader:
            if header:
                header = False
                continue
            results.append(load_result_from_row(row))

    plot_graph(results)
