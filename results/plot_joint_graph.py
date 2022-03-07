import argparse
import numpy as np
import plot
import plot_ffta

import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

def plot_lines(mode, fftw, ffta, powerquad):
    fig, ax = plt.subplots()
    ax.set_xscale('log', base=2)
    if mode == 'all':
        ax.set_yscale('log', base=10)
        plt.ylabel("Relative Performance (log)")
    elif mode == 'slow':
        plt.ylim([0, 30])
        plt.ylabel("Relative Performance")
    else:
        plt.ylabel("Relative Performance")

    plt.xlabel("Input Size")
    plt.xlim([4, 2**14])

    for i in range(len(fftw)):
        ffta_keys = sorted(ffta[i].keys())
        ffta_values = [ffta[i][k] for k in ffta_keys]
        plt.plot(ffta_keys, ffta_values, '--', color='green')

        fftw_keys = sorted(fftw[i].keys())
        fftw_values = [fftw[i][k] for k in fftw_keys]
        plt.plot(fftw_keys, fftw_values, '-.', color='orange')

        pq_keys = sorted(powerquad[i].keys())
        pq_values = [powerquad[i][k] for k in pq_keys]
        plt.plot(pq_keys, pq_values, ':', color='red')

    custom_lines = [
            Line2D([0], [0], linestyle='--', color='green', label='FFTA'),
            Line2D([0], [0], linestyle='-.', color='orange', label='FFTW'),
            Line2D([0], [0], linestyle=':', color='red', label='PowerQuad')
            ]
    ax.legend(handles=custom_lines)
    plt.savefig(mode + "_output.eps")

def compute_ratio(orig, accel, key):
    if key in orig and key in accel and orig[key] != 0 and accel[key] != 0:
        return orig[key] / accel[key]
    else:
        return None

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("mode")
    parser.add_argument("Folders", nargs='+')

    fftw_folder_orig = 'accelerated_fftw_orig_results'
    fftw_folder_accel = 'accelerated_fftw_results'

    ffta_file_orig = 'ffta_results/UnAcceleratedResults'
    ffta_file_accel = 'ffta_results/AcceleratedResults'

    powerquad_file_orig = 'powerquad_results/UnAcceleratedResults'
    powerquad_file_accel = 'powerquad_results/AcceleratedResults'

    args = parser.parse_args()

    ffta = []
    fftw = []
    powerquad = []

    for folder in args.Folders:
        print ("Loking at folder " + str(folder))

        # Get FFTW numbers.
        fftw_orig = plot.load_result_maps(folder, fftw_folder_orig, "json_out")
        fftw_accel = plot.load_result_maps(folder, fftw_folder_accel, "json_out")
        fftw_line = {}
        for k in fftw_orig.keys():
            res = compute_ratio(fftw_orig, fftw_accel, k)
            if res:
                fftw_line[k] = res

        # Get FFTA numbers.
        ffta_orig, _ = plot_ffta.read_file(folder + "/" + ffta_file_orig)
        ffta_accel, _ = plot_ffta.read_file(folder + "/" + ffta_file_accel)
        ffta_line = {}
        for k in ffta_orig.keys():
            res = compute_ratio(ffta_orig, ffta_accel, k)
            if res:
                ffta_line[k] = res

        # Get PowerQuad numbers.
        powerquad_orig, _ = plot_ffta.read_file(folder + "/" + powerquad_file_orig)
        powerquad_accel, _ = plot_ffta.read_file(folder + "/" + powerquad_file_accel)
        powerquad_line = {}
        for k in powerquad_orig.keys():
            res = compute_ratio(powerquad_orig, powerquad_accel, k)
            if res:
                powerquad_line[k] = res

        # add the lines from each.
        fftw.append(fftw_line)
        ffta.append(ffta_line)
        powerquad.append(powerquad_line)

    plot_lines(args.mode, fftw, ffta, powerquad)
