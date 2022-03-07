import sh
from glob import glob
import os
from collections import defaultdict
from collections import OrderedDict
import pandas as pd
import numpy as np

PATH = 'output'
NAME_PATTERN = 'NEW'


def main():
    nested_dict = lambda: defaultdict(nested_dict)
    results = nested_dict()
    metrics = set()
    for line in sh.grep(glob(os.path.join(PATH, f'{NAME_PATTERN}*/train.log')), '-e', 'Test').splitlines():
        if 'grad_fn' in line or 'length' in line:
            continue
        data = int(line.split('-')[1])
        cv = int(line.split('-')[3])
        metric = '_'.join(line.split(':')[-2].split()[:-1])
        metrics.add(metric)
        value = float(line.split()[-1])
        if value == -1:
            value = np.nan
        results[data][cv][metric] = value

    # Workaround for getting FFT top-3 accuracy (log said validation, not test)
    #for line in sh.grep(glob(os.path.join(PATH, f'{NAME_PATTERN}*/train.log')), '-A', 1, '-e', 'Test accuracy').splitlines():
    #    if 'FFT' not in line:
    #        continue
    #    data = int(line.split('-')[1])
    #    cv = int(line.split('-')[3])
    #    metric = '_'.join(line.split(':')[-2].split()[:-1])
    #    metrics.add(metric)
    #    value = float(line.split()[-1])
    #    results[data][cv][metric] = value

    results = OrderedDict(sorted(results.items(), key=lambda t: t[0]))
    for k in results:
        results[k] = OrderedDict(sorted(results[k].items(), key=lambda t: t[0]))
    table = {'instances_per_class': [], 'cv_index': []}
    for metric in metrics:
        table[metric] = []
    for data in results:
        for cv in results[data]:
            table['instances_per_class'].append(data)
            table['cv_index'].append(cv)
            for metric in metrics:
                table[metric].append(results[data][cv][metric])
    df = pd.DataFrame.from_dict(table)
    df.to_csv('results3.csv', index=False, )


if __name__ == '__main__':
    main()
