import matplotlib.pyplot as plt
import numpy as np
import argparse
import glob
import os

def parse_filename(fname):
    # format is: <input size>.json_out
    number = int(os.path.basename(fname).split(".")[0])
    return number

def load_from_file(fname):
    times = []
    with open(fname) as f:
        for line in f.readlines():
            time=float(line.strip())
            times.append(time)

    # Get a median.  TODO -- get CI.
    median = np.median(times)
    print("For file " + fname + " have median " + str(median))
    return median

def get_lines_from(set):
    lists = sorted(set.items())
    x,y = zip(*lists)

    return x, y

def plot(group, lines, names, style):
    plt.clf()
    plt.cla()
    fix, ax = plt.subplots()
    ax.set_xscale('log', base=2)
    if group == 'all' and style == 'speedup':
        ## DFTs are /really/ slow, so show on log.
        ax.set_yscale('log', base=10)
    for lineset, name in zip(lines, names):
        if len(lineset) == 0:
            print (name + " has empty lineset")
            print (names)
            print (lines)
            raise error
        x, y = get_lines_from(lineset)
        plt.plot(x, y, label=name)
        minx, maxx = min(x), max(x)
        plt.xlim([minx, maxx])

    if style == 'speedup':
        # plt.plot([minx, maxx], [1.0, 1.0], label="Speedup Threshold")
        appendix = "(log)" if group == "all" else ""
        plt.ylabel("Speedup Ratio " + appendix)
        plt.title("Speed Comparison Across Different Sizes")
        plt.xlabel("Input size")
    elif style == 'overhead':
        # plt.plot([minx, maxx], [1.0, 1.0], label="Low Overhead Threshold")
        plt.ylabel("Overhead")
        plt.title("Overhead of FACC-Generated Code Across Different Sizes")
        plt.xlabel("Input size")
    else:
        error("Unknwwn sytle")

    # plt.legend()

    plt.savefig(style + "_" + group + "_output.eps")
    plt.close()

def load_result_maps(base_folder, folder, postfix):
    files = glob.glob(base_folder + "/" + folder + "/*." + postfix)
    results = {}

    for file in files:
        # get the number:
        number = parse_filename(file)
        values = load_from_file(file)
        results[number] = np.array(values)

    return results

if __name__ == "__main__":
    plt.clf()
    parser = argparse.ArgumentParser(description="FFT Result Plotter")

    parser.add_argument("group")
    parser.add_argument("OriginalResultsFolder")
    parser.add_argument("AcceleratedResultsFolder")
    parser.add_argument("Folders", nargs='+')

    args = parser.parse_args()

    lines = []
    names = []
    for folder in args.Folders:
        print ("Looking at folder " + str(folder))
        v1 = load_result_maps(folder, args.OriginalResultsFolder, "json_out")
        v2 = load_result_maps(folder, args.AcceleratedResultsFolder, "json_out")
        res = {}
        for k in v1.keys():
            if k in v2.keys():
                if v2[k] != 0 and v1[k] != 0:
                    res[k] = v1[k] / v2[k]
                else:
                    print ("Skipping key " + str(k) + " due to zero value")
                    # Skip if div by 0
            else:
                print ("Warning: did not get a result for both original and accelerated")

        print ("Res is "  + str(res))
        lines.append(res)
        names.append(folder)

    plot(args.group, lines, names, 'speedup')

    # Plot the overhead graph
    lines = []
    names = []
    for folder in args.Folders:
        v1 = load_result_maps(folder, args.AcceleratedResultsFolder, "json_out")
        v2 = load_result_maps(folder, args.AcceleratedResultsFolder, "json_acctime_out")
        v3 = load_result_maps(folder, args.OriginalResultsFolder, "json_out")
        res = {}
        for k in v1.keys():
            # Do the subtraction because v2 actually measures the time /in/ the accelerator,
            # which is the inverse of the overhead.
            if v3[k] != 0 and v2[k] != 0 and v3[k] != 0:
                res[k] = (v1[k] - v2[k]) / v2[k]

        lines.append(res)
        names.append(folder)

    plot(args.group, lines, names, 'overhead')
    print ("Done!")
