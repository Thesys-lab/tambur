#!/usr/bin/env python3

import os
import sys
import argparse
import json
import numpy as np
import pandas as pd
import copy
import argparse
import json
import numpy as np

from plot_common import (plt, pretty_name, pretty_color, pretty_hatch,
                          pretty_linestyle, baseline,
                          get_mean_bandwidth_overhead,
                          get_non_recoverable_frames,
                          is_proper_log_dir)

def get_plot_data(args, tau):

    coding_schemes = []
    non_recoverable_frames_list = []
    bandwidth_overhead = []

    for fname in os.scandir(args.log):
        fsplit = fname.name.split('.')

        if is_proper_log_dir(fname, args.config):
            coding_scheme = ''.join(fsplit[2:])
            if coding_scheme in args.skip:
                continue
            use_tau = tau
            if (coding_scheme == baseline) or (coding_scheme == pretty_name[baseline]) or (pretty_name[coding_scheme] == pretty_name[baseline]):
              use_tau = 0

            coding_schemes.append(coding_scheme)
            tr_non_recoverable_frames = []
            tr_bandwidth_overhead = []
            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue

                nrf = get_non_recoverable_frames(trace_log, use_tau, args.FECOnly)
                mbw = get_mean_bandwidth_overhead(trace_log, use_tau, args.FECOnly)
                if nrf != -1 and mbw != 0:
                    tr_bandwidth_overhead.append(mbw)
                    tr_non_recoverable_frames.append(nrf)

            tr_bandwidth_overhead = np.asarray(tr_bandwidth_overhead)
            tr_non_recoverable_frames = np.asarray(tr_non_recoverable_frames)
            if args.percentile:
                comb = zip(tr_non_recoverable_frames, tr_bandwidth_overhead)
                sort_comb = sorted(comb, key=lambda tup: tup[0])
                pidx = int((float(args.percentile)/100.0)*tr_bandwidth_overhead.size)
                bandwidth_overhead.append(sort_comb[pidx][1])
                non_recoverable_frames_list.append(sort_comb[pidx][0])
            else:
                bandwidth_overhead.append(np.mean(tr_bandwidth_overhead))
                non_recoverable_frames_list.append(np.mean(tr_non_recoverable_frames))


    return coding_schemes, non_recoverable_frames_list, bandwidth_overhead

def plot_data(args, coding_schemes, non_recoverable_frames_list, bandwidth_overhead):
    bandwidth_overhead = np.asarray(bandwidth_overhead)
    non_recoverable_frames_list = np.asarray(non_recoverable_frames_list)

    baselineIdx = coding_schemes.index(baseline)
    bandwidth_overhead = (100.0*(bandwidth_overhead[baselineIdx] - bandwidth_overhead))/bandwidth_overhead[baselineIdx]
    non_recoverable_frames_reduction = (100.0*(non_recoverable_frames_list[baselineIdx] - non_recoverable_frames_list))/non_recoverable_frames_list[baselineIdx]

    fig, ax = plt.subplots()
    ax.scatter(non_recoverable_frames_reduction, bandwidth_overhead)

    for i, txt in enumerate(coding_schemes):
        ax.annotate(txt, (non_recoverable_frames_reduction[i], bandwidth_overhead[i]))

    if args.percentile:
        ax.set_xlabel(f'''p{args.percentile} non-recoverable frames reduction (%)''')
        ax.set_ylabel(f'''p{args.percentile} bandwidth overhead reduction (%)''')
    else:
        ax.set_xlabel('Total non-recoverable frames reduction (%)')
        ax.set_ylabel('Total bandwidth overhead reduction (%)')

    fig.savefig(args.output, bbox_inches='tight')
    print('Saved figure to', args.output)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument("--percentile", required=False, help="Plot the given percentile. By default it will plot mean")
    parser.add_argument("--skip", required=False, action='append', default=[])
    parser.add_argument('--FECOnly', default=True, help='only non-recoverable frames when FEC used')

    args = parser.parse_args()
    with open(args.config, 'r') as f:
      d = json.load(f)
    assert(len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])

    args.config = os.path.basename(args.config)
    log_path = args.output[0:-4] + ".json"
    log_exists = os.path.exists(log_path)
    if (log_exists):
        with open(log_path, 'r') as f:
            data = json.load(f)
        coding_schemes = data["coding_schemes"]
        non_recoverable_frames_list = data["non_recoverable_frames_list"]
        bandwidth_overhead = data["bandwidth_overhead"]
    else:
        coding_schemes, non_recoverable_frames_list, bandwidth_overhead = get_plot_data(args, tau)
        conf = {}
        conf["coding_schemes"] = coding_schemes
        conf["non_recoverable_frames_list"] = non_recoverable_frames_list
        conf["bandwidth_overhead"] = bandwidth_overhead
        with open(log_path, 'w') as f:
            json.dump(conf, f)
    plot_data(args, coding_schemes, non_recoverable_frames_list, bandwidth_overhead)

if __name__ == '__main__':
    main()
