import argparse
import matplotlib.pyplot as plt
import matplotlib.ticker as tkr
import plot
import plot_ffta
import numpy as np

def geomean_of(l):
    cat = np.concatenate(l)
    a = cat
    return a.prod()**(1.0/len(a))

def get_ticks_for_graph(i):
    if i == 0:
        return tkr.AutoLocator()
    elif i == 1:
        return tkr.AutoLocator()
    else:
        return tkr.LogLocator()

def plot_graph(fftw_results, ffta_results, powerquad_results):
    gridspec = {
            'width_ratios': [len(x) for x in fftw_results]
            }
    fix, (ax1, ax2, ax3) = plt.subplots(1, len(fftw_results), figsize=(8,3), gridspec_kw = gridspec)
    ax3.set_yscale('log', base=10)
    ax3.set_ylim([0.1, 100000])

    axes = [ax1, ax2, ax3]
    onetkr = tkr.FixedLocator([0.999])

    last_maxx = 0
    for i in range(0, len(fftw_results)):
        fftw_group = fftw_results[i]
        ffta_group = ffta_results[i]
        powerquad_group = powerquad_results[i]
        axi = axes[i]
        minx = last_maxx
        maxx = minx + len(fftw_group)
        last_maxx = maxx

        xpos = np.arange(0, len(fftw_group))
        print (len(ffta_group))
        print (len(fftw_group))
        print (len(xpos))
        width = 0.25
        axi.bar(xpos, fftw_group,  width,  label='FFTW', color='orange')
        axi.bar(xpos + width, powerquad_group, width, label='PowerQuad', color='red', hatch='.')
        axi.bar(xpos + width * 2, ffta_group, width, label='FFTA', color='green', hatch='-')
        axi.set_xticks(xpos + width)
        axi.set_xticklabels(range(minx, maxx))
        axi.yaxis.set_minor_locator(onetkr)
        axi.yaxis.set_major_locator(get_ticks_for_graph(i))
        axi.grid(axis='y', which='minor')

    ax3.legend()
    ax2.set_xlabel('Project Number')
    ax1.set_ylabel('Performance Relative to\nSystem Baseline')

    plt.tight_layout()
    plt.savefig('barplot_speedup.eps')

def plot_adi_graph(accres, dspres):
    # A bunch of this is duplicated fro the above call, but cba to make it better,
    # I'm very tired.
    gridspec =  {
            'width_ratios': [len(x) for x in accres]
            }
    fix, (ax1, ax2, ax3) = plt.subplots(1, len(accres), figsize=(8, 3), gridspec_kw = gridspec)
    ax3.set_yscale('log', base=10)
    ax3.set_ylim([0.1, 100000])

    axes = [ax1, ax2, ax3]
    last_maxx = 0
    onetkr = tkr.FixedLocator([0.999])
    for i in range(0, len(accres)):
        acc_group = accres[i]
        dsp_group = dspres[i]
        axi = axes[i]

        minx = last_maxx
        maxx = minx + len(acc_group)
        last_maxx = maxx
        # On the first bar, IDL gets the right answer, because we built the pattern for it.
        width = 0.25

        if minx == 0:
            axi.bar(0 + 2 * width, acc_group[0], width, label='IDL', color='blue')

        xpos = np.arange(0, len(acc_group))
        axi.bar(xpos, dsp_group, width, label='Neural Classifier', color='red', hatch='-')
        axi.bar(xpos + width, acc_group, width, label='FACC', color='green', hatch='-')
        if i != 3:
            # hack to get idl in the legend.
            axi.bar(xpos + width, [0] * len(acc_group), label='IDL', color='blue')
        axi.set_xticks(xpos + width)
        axi.set_xticklabels(range(minx, maxx))
        axi.yaxis.set_minor_locator(onetkr)
        axi.yaxis.set_major_locator(get_ticks_for_graph(i))
        axi.grid(axis='y', which='minor')

    ax2.legend()
    ax2.set_xlabel('Project Number')
    ax1.set_ylabel('Performance Relative to Cortex-A5')

    plt.tight_layout()
    plt.savefig('sc589_system_speedup.eps')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('size', type=int)
    parser.add_argument('directories', nargs='+')
    args = parser.parse_args()

    ffta_results = []
    ffta_dsp_results = []
    fftw_results = []
    powerquad_results = []

    i = 0
    for folder in args.directories:
        print ("Folder " + folder)
        print ("is project " + str(i))
        i += 1
        # Get the FFTW results:
        fftw_orig = plot.load_result_maps(folder, 'accelerated_fftw_orig_results', "json_out")
        fftw_acc = plot.load_result_maps(folder, 'accelerated_fftw_results', "json_out")

        if args.size not in fftw_orig:
            size = max(fftw_orig.keys())
            print("For project failed to get the sizes")
            print (folder)
            print ("Using size ", size)
        else:
            size = args.size

        fftw_results.append(fftw_orig[size] / fftw_acc[size])
        print ("Value used is ", fftw_results[-1])

        # Get the FFTA results:
        ffta_orig, _ = plot_ffta.read_file(folder + "/ffta_results/" + "UnAcceleratedResults")
        ffta_acc, _ = plot_ffta.read_file(folder + "/ffta_results/" + "AcceleratedResults")
        ffta_dsp, _ = plot_ffta.read_file(folder + "/ffta_results/" + "SC589Results")

        if args.size not in ffta_orig:
            size = max(ffta_orig.keys())
            print("For project failed to get the sizes")
            print (folder)
            print ("Using size ", size)
        else:
            size = args.size

        ffta_results.append(np.median(ffta_orig[size] / ffta_acc[size]))
        ffta_dsp_results.append(np.median(ffta_orig[size] / ffta_dsp[size]))

        # Get PowerQuad results:
        powerquad_orig, _ = plot_ffta.read_file(folder + "/powerquad_results/UnAcceleratedResults")
        powerquad_acc, _ = plot_ffta.read_file(folder + "/powerquad_results/AcceleratedResults")

        if len(powerquad_orig) > 0:
            if args.size not in powerquad_orig:
                size = max(powerquad_orig.keys())
                print ("For project failed to get the sizes")
                print (folder)
                print ("Using size ", size)
            else:
                size = args.size

            powerquad_results.append(np.median(powerquad_orig[size] / powerquad_acc[size]))
            print ("Value used is ", ffta_results[-1])
        else:
            print("No powerquad results found for ", folder)
            powerquad_results.append(0.0)


    # Sort the values:
    res = []
    for i in range(len(args.directories)):
        res.append(
                (
                ffta_results[i],
                ffta_dsp_results[i],
                fftw_results[i],
                powerquad_results[i],
                args.directories[i]
                )
        )

    res = sorted(res, key=lambda x: max([x[0], x[1], x[2], x[3]]))
    # Due to a great fuckup,the order changed after we'd arbitrayily picked project 0
    # which is not project 1.  Just swap those two so that it's clear why we picked
    # project 0 for various idl examples.
    res[0], res[1] = res[1], res[0]

    fftw_results = []
    ffta_results = []
    ffta_dsp_results = []
    powerquad_results = []
    dirs = []
    i = 0
    for r in res:
        if i == 0 or i == 4 or i == 11:
            # That's where the new groups start --- a better way to do this would be to compute the actual thresholds that are being used for this division.
            ffta_results.append([])
            ffta_dsp_results.append([])
            fftw_results.append([])
            powerquad_results.append([])
            dirs.append([])

        ffta_results[-1].append(r[0])
        ffta_dsp_results[-1].append(r[1])
        fftw_results[-1].append(r[2])
        powerquad_results[-1].append(r[3])
        dirs[-1].append(r[4])

        i += 1

    plot_graph(fftw_results, ffta_results, powerquad_results)
    plot_adi_graph(ffta_results, ffta_dsp_results)
    print ("Order of folders")
    print (dirs)

    # compute the geomeans and print that too:
    ffta_geomean = geomean_of(ffta_results)
    sc589geomean = geomean_of(ffta_dsp_results)
    fftw_geomean = geomean_of(fftw_results)
    powerquad_geomean = geomean_of(powerquad_results)

    print ("FFTA geomean", ffta_geomean)
    print ("SC589 Geomean", sc589geomean)
    print ("FFTW geomean", fftw_geomean)
    print ("Powerquad geomean", powerquad_geomean)
