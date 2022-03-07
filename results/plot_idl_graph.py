import matplotlib.pyplot as plt
import matplotlib.ticker as tkr
import argparse
import csv

class Datum:
    def __init__(self, name, xs, ys, maxs):
        self.name = name
        self.xs = []
        self.ys = []

        for i in range(len(xs)):
            self.xs.append(int(xs[i]))
            self.ys.append(float(ys[i]) / float(maxs[i]))

def plot(fs):
    plt.figure(figsize=(6, 3))
    for f in fs:
        plt.plot(f.xs, f.ys, label=f.name)

    plt.xlabel("Number of Lines of IDL")
    plt.ylabel("Fraction of FFTs Matched")
    plt.ylim([0, 1])
    plt.xlim([0, 70])
    loc = tkr.FixedLocator([0.2, 0.4, 0.6, 0.8])
    ax = plt.gca()
    ax.yaxis.set_major_locator(loc)
    ax.grid(axis='y', which='major')
    plt.legend()
    plt.tight_layout()
    plt.savefig("idl_graph.eps")

def load_files(files, names):
    lines = []
    for i in range(len(names)):
        f = files[i]
        name = names[i]
        print ("Loading ", f)
        with open(f) as dat:
            reader = csv.reader(dat)
            next(reader, None) # skip headers

            xs = []
            ys = []
            maxs = []
            for line in reader:
                print (line)
                xs.append(line[0])
                ys.append(line[1])
                maxs.append(line[2])
        lines.append(Datum(name, xs, ys, maxs))
    return lines

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('DataFiles', nargs='+')

    args = parser.parse_args()
    files, names = [], []
    for i in range(len(args.DataFiles)):
        if i % 2 == 0:
            files.append(args.DataFiles[i])
        else:
            names.append(args.DataFiles[i])

    datas = load_files(files, names)
    plot(datas)
