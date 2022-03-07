import matplotlib.pyplot as plt
import matplotlib.ticker as tkr
import argparse
import os
import numpy as np
import re
import plot_compile_times

def plot(ccounts):
    plt.clf()
    plt.figure(figsize=(6, 3))
    for c in ccounts:
        x, y = c.get_cdf()
        color = plot_compile_times.get_color(c.name)
        style = plot_compile_times.get_linestyle(c.name)

        plt.plot(x, y, label=c.name, color=color, linestyle=style)

    plt.legend()
    loc = tkr.FixedLocator([0.2, 0.4, 0.6, 0.8])
    axes = plt.gca()
    axes.set_ylim([0.0, 1.001])
    axes.yaxis.set_major_locator(loc)
    axes.grid(which='major', axis='y')
    axes.set_xscale('log')
    axes.set_xlim([1, 1000])

    plt.xlabel("Number of Binding Candidates")
    plt.ylabel("CDF")
    plt.tight_layout()

    plt.savefig("candidates.eps")

class CCount(object):
    def __init__(self, name, path):
        self.name = name
        self.path = path

    def get_counts(self):
        counts = []
        with open (self.path) as f:
            for line in f.readlines():
                if "Number of programs from these pairs" in line:
                    cands = int(re.split(' ', line)[7])
                    counts.append(cands)
        return counts

    def get_cdf(self):
        cands = np.array(sorted(self.get_counts()))
        y_var = np.array(range(0, len(cands)))
        y_var = y_var / (len(cands) - 1)
        
        return cands, y_var

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('folder')
    args = parser.parse_args()

    ccounts_files = []
    # That is better vv
    # for path, sdirs, files in os.walk(args.folder):
    for name in ['FFTA', 'FFTW', 'PowerQuad']:
        pname = os.path.join(args.folder, name)
        ccounts_files.append(CCount(name, pname))

    plot(ccounts_files)
