#!/usr/bin/env python3

from calendar import c
import os
import copy
import sys
import argparse
import json
import numpy as np
import pandas as pd

import argparse
import json
import numpy as np

from plot_common import (plt, pretty_name, pretty_color,pretty_hatch,
                          pretty_linestyle, cdf_data_helper, baseline,
                          get_mean_bandwidth_overhead,
                          is_proper_log_dir)

plt.rcParams['font.size'] = 18
plt.rcParams['axes.labelsize'] = 18
plt.rcParams['legend.fontsize'] = 16

def get_cdf_data(args, tau):
    bandwidth_overhead = {}
    coding_schemes = []
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
            tr_bandwidth_overhead = []
            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue
                mbw = get_mean_bandwidth_overhead(trace_log, use_tau, args.FECOnly)
                if mbw > 0:
                    tr_bandwidth_overhead.append(mbw)
            tr_bandwidth_overhead.sort()
            bandwidth_overhead[coding_scheme] = np.asarray(tr_bandwidth_overhead)
    return bandwidth_overhead, coding_schemes

def plot_data(args, cdf_data, coding_schemes):
    fig, ax = plt.subplots()
    ax.set_box_aspect(1/1.5)

    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        filtered_data[code] = [[], []]

        for x, y in zip(cdf_data[code][0], cdf_data[code][1]):
            if y >= args.ymin and y <= args.ymax:
                filtered_data[code][0].append(x)
                filtered_data[code][1].append(y)

    for code in coding_schemes:
        ax.plot(filtered_data[code][0],
                filtered_data[code][1],
                label=pretty_name[code],
                color=pretty_color[code],
                linestyle=pretty_linestyle[code],
                linewidth=2,
                clip_on=False)

    ax.set_ylim(args.ymin, args.ymax)

    ax.legend(loc='upper left')
    ax.set_xlabel('Bandwidth overhead per trace (%)')
    ax.set_ylabel('CDF')

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)

    fig.savefig(args.output[0:-4] + "_{}_{}.svg".format(args.ymin, args.ymax), bbox_inches='tight')
    print('Saved figure to', args.output[0:-4] + "_{}_{}.svg".format(args.ymin, args.ymax))
    fig.savefig(args.output[0:-4] + "_{}_{}.pdf".format(args.ymin, args.ymax), bbox_inches='tight')
    print('Saved figure to', args.output[0:-4] + "_{}_{}.pdf".format(args.ymin, args.ymax))

def plot_bar_data(args, data, coding_schemes, percentiles):
    fig, ax = plt.subplots()
    width = 0.2 * 4.0 / 5
    start_pos = 0
    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        filtered_data[code] = []
        filtered_data[code +"_mean"] = np.mean(data[code])
        for p in percentiles:
            idx = int((p/100.0)*data[code].size)
            filtered_data[code].append(data[code][idx])
    with open (args.output[:-4]  +
            "_data_bar_{}_{}.json".format(args.ymin, args.ymax), 'w') as f:
        json.dump(filtered_data, f)

    xticklabels = [str(percent) for percent in percentiles]
    xticks = [i+2*width for i in range(len(percentiles))]
    xdata = np.arange(len(percentiles))
    for code in coding_schemes:
        ax.bar(xdata + start_pos,
            filtered_data[code],
            width=width,
            label=pretty_name[code],
            color=pretty_color[code],
            alpha=0.7,
            clip_on=False,
            linestyle=pretty_linestyle[code], hatch=pretty_hatch[code])
        start_pos += width

    # avoid repeated labels
    handles, labels = fig.gca().get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(),
            loc='upper right')

    ax.set_xlabel('Percentile over videos')
    ax.set_ylabel('Percent overhead of bandwidth')

    ax.set_xticks(xticks)
    ax.tick_params(axis='x', length=0)
    ax.set_xticklabels(xticklabels)

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['bottom'].set_visible(False)

    ax.spines['top'].set_visible(False)
    fig.savefig(args.output[:-4]  +  "_bar_{}_{}.svg".format(args.ymin, args.ymax), bbox_inches='tight')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument('--ymin', default=0, type=float,
                        help='min Y-axis value to plot')
    parser.add_argument('--FECOnly', default=True, help='only non-recoverable frames when FEC used')
    parser.add_argument('--ymax', default=1, type=float,
                        help='max Y-axis value to plot')
    parser.add_argument("--skip", required=False, action='append', default=[])

    args = parser.parse_args()
    args = parser.parse_args()
    with open(args.config, 'r') as f:
      d = json.load(f)
    assert(len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])
    percentiles = [5, 10, 15, 20] + [25, 50, 75, 90]
    args.config = os.path.basename(args.config)
    log_path = args.output[0:-4] + ".json"
    log_exists = os.path.exists(log_path)
    if log_exists:
        with open(log_path, 'r') as f:
            cdf_data = json.load(f)
        coding_schemes = cdf_data["coding_schemes"]
        del cdf_data["coding_schemes"]
        for key in cdf_data.keys():
            cdf_data[key] = np.asarray(cdf_data[key])
    else:
        cdf_data, coding_schemes = get_cdf_data(args, tau)
        conf = copy.deepcopy(cdf_data)
        for key in conf.keys():
            conf[key] = conf[key].tolist()
        conf["coding_schemes"] = coding_schemes
        with open(log_path, 'w') as f:
            json.dump(conf, f)
    tmp = coding_schemes
    coding_schemes = []
    for item in tmp:
        if "Reed" in item:
            coding_schemes.append(item)
    for item in tmp:
        if "point" in item:
            coding_schemes.append(item)
    coding_schemes.append("StreamingCode_3")
    for item in tmp:
        if "high" in item:
            coding_schemes.append(item)

    plot_data(args, cdf_data_helper(cdf_data), coding_schemes)
    plot_bar_data(args, cdf_data, coding_schemes, percentiles)

if __name__ == '__main__':
    main()
