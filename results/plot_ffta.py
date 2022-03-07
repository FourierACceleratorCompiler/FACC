import argparse
import numpy as np
import plot as plotter # other python file here
import os

def splitlist(l, splitlen):
    i = 0
    rlist = []
    tlist = []
    for item in l:
        if i == splitlen:
            rlist.append(tlist)
            tlist = []
            i = 0
        tlist.append(item)
        i += 1

    if tlist is not []:
        rlist.append(tlist)

    return rlist

def read_file(fname):
    if not os.path.exists(fname):
        print ("File doesn't exist, skipping. ")
        print (fname)
        return {}, {}

    print ("Looking at file " , fname)

    with open(fname) as f:
        linegroups = splitlist(f.readlines(), 4)

    total_times = {}
    accelerator_times = {}
    for group in linegroups:
        # Get the runsize:
        # Runs finished for size X..
        runsize = int(group[3].split(' ')[4])
        print ("Lookng at runsize", runsize)

        #TIme: X...
        time = float(group[1].split(' ')[1])

        #AccTime: X...
        acc_time = float(group[2].split(' ')[1])

        if runsize in total_times:
            np.append(total_times[runsize], time)
            np.append(accelerator_times[runsize], acc_time)
        else:
            total_times[runsize] = np.array([time])
            accelerator_times[runsize] = np.array([acc_time])

    return total_times, accelerator_times

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="FFT Result Plotter")

    parser.add_argument("graph_type")
    parser.add_argument("OriginalResultsFile")
    parser.add_argument("AcceleratedResultsFile")
    parser.add_argument("Folders", nargs='+')

    args = parser.parse_args()

    lines = []
    names = []
    num = 0

    # Plot the speedup:
    for folder in args.Folders:
        print ("Looking at folder " + str(folder))
        num += 1
        original_results, _ = read_file(folder + "/" + args.OriginalResultsFile)
        acc_results, acc_time = read_file(folder + "/" + args.AcceleratedResultsFile)

        res = {}
        for k in acc_results.keys():
            ## FFTA results are determinsitic, so just take the
            # head.
            res[k] = (original_results[k] / acc_results[k])

        lines.append(res)
        names.append("Project " + str(folder))

    plotter.plot(args.graph_type, lines, names, 'speedup')

    # Plot the overhead:
    num = 0
    lines = []
    names = []
    for folder in args.Folders:
        print ("Looking at project " + str(folder))
        num += 1
        orig_res, _ = read_file(folder + "/" + args.OriginalResultsFile)
        acc_res, acc_time = read_file(folder + "/" + args.AcceleratedResultsFile)

        res = {}
        for k in acc_res.keys():
            # time spent in wrapper code / time spent in accelerator.
            res[k] = (acc_res[k] - acc_time[k]) / acc_time[k]
        lines.append(res)
        names.append("Project " + str(folder))
    plotter.plot(args.graph_type, lines, names, 'overhead')
