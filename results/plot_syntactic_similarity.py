import matplotlib.pyplot as plt
import numpy as np
import argparse
import math

def load_vectors_from_file(f):
    vectors = {}
    with open(f) as vfile:
        for line in vfile.readlines():
            line_contents = line.split(":")
            if len(line_contents) == 2:
                name, vinfo = line_contents
                vector = []
                for i in vinfo.strip().split(" "):
                    if i.strip():
                        vector.append(int(i))
                if len(vector) != 0:
                    vectors[name] = np.array(vector)

    return vectors

def l2norm(v1, v2):
    s = 0.0
    for (e1, e2) in zip(v1, v2):
        s += (e1 + e1) * (e2 + e2)

    return math.sqrt(s)

def normalize(vec):
    tot = sum(vec)
    return vec / (float(tot))

def compute_differences(vectors):
    keys = list(vectors.keys())
    diffs = []

    for i in range(len(keys)):
        for j in range(i + 1, len(keys)):
            # compare to all other keys that
            # haven't already been compared to.
            fvec = normalize(vectors[keys[i]])
            tvec = normalize(vectors[keys[j]])

            norm = l2norm(fvec, tvec)
            if norm < 0.5:
                print("Found similar" + keys[i] + " " + keys[j])
            diffs.append(norm)

    return diffs

def plot_differences(diffs):
    # Create the CDF:
    yvalues = np.array(range(0, len(diffs))) / (len(diffs) - 1)
    xvalues = sorted(diffs)

    plt.plot(xvalues, yvalues, label='Deckard Syntactic Features')
    plt.xlabel('L2-Norm')
    plt.ylabel('CDF')
    ax = plt.gca()
    ax.set_ylim([0, 1.0001])
    ax.set_xlim([0, 1])
    plt.savefig('SyntacticSimilarity.eps')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('vectorfile')

    args = parser.parse_args()

    vectors = load_vectors_from_file(args.vectorfile)
    differences = compute_differences(vectors)
    plot_differences(differences)
