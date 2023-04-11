#!/usr/bin/env python3

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

from plot_common import (plt, pretty_name, pretty_color, pretty_hatch,
                        pretty_linestyle, cdf_data_helper, baseline,
                         is_proper_log_dir, get_freezes_durations)

plt.rcParams['font.size'] = 16
plt.rcParams['legend.fontsize'] = 16


def get_cdf_data(args, tau):
    all_freezes = {}
    all_freeze_durations = {}
    all_average_freeze_durations = {}
    all_median_freeze_durations = {}
    coding_schemes = []

    for fname in os.scandir(args.log):
        fsplit = fname.name.split('.')

        if is_proper_log_dir(fname, args.config):
            coding_scheme = ''.join(fsplit[2:])
            if coding_scheme in args.skip:
                continue
            coding_schemes.append(coding_scheme)
            use_tau = tau
            if (coding_scheme == baseline) or (coding_scheme == pretty_name[baseline]) or (pretty_name[coding_scheme] == pretty_name[baseline]):
              use_tau = 0

            all_freezes[coding_scheme] = []
            all_freeze_durations[coding_scheme] = []
            all_average_freeze_durations[coding_scheme] = []
            all_median_freeze_durations[coding_scheme] = []

            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue
                freeze_durations, freezes, average_freeze_durations, median_freeze_durations = get_freezes_durations(trace_log, use_tau, args.FECOnly)
                all_freezes[coding_scheme].append(freezes)
                all_freeze_durations[coding_scheme].append(freeze_durations)
                all_average_freeze_durations[coding_scheme].append(average_freeze_durations)
                all_median_freeze_durations[coding_scheme].append(median_freeze_durations)
            all_freezes[coding_scheme].sort()
            all_freeze_durations[coding_scheme].sort()
            all_average_freeze_durations[coding_scheme].sort()
            all_median_freeze_durations[coding_scheme].sort()

    return all_freezes, all_freeze_durations, all_average_freeze_durations, all_median_freeze_durations, coding_schemes

def plot_data(args, freezes, coding_schemes, extra_sv_name, xlabel, clipped = False, sv = True):
    multiplier = 1.0
    if "%" in xlabel:
        multiplier = 100.0
    fig, ax = plt.subplots()
    ax.set_box_aspect(1/1.8)

    ignore = []
    for code in coding_schemes:
        freezes[code] = np.asarray(freezes[code])
        if freezes[code].size == 0:
            del freezes[code]
            ignore.append(code)
    cdf_data = cdf_data_helper(freezes)

    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        if code in ignore:
            continue
        filtered_data[code] = [[], []]
        for x, y in zip(cdf_data[code][0], cdf_data[code][1]):
            if y >= args.ymin and y <= args.ymax:
                if (not clipped) or (args.xmax < 0) or (x <= args.xmax):
                    filtered_data[code][0].append(x * multiplier)
                else:
                    filtered_data[code][0].append(args.xmax * multiplier)
                filtered_data[code][1].append(y)

    for code in coding_schemes:
        if code in ignore:
            continue
        ax.plot(filtered_data[code][0],
                filtered_data[code][1],
                label=pretty_name[code],
                color=pretty_color[code],
                linestyle=pretty_linestyle[code],
                linewidth=2,
                clip_on=False)

    ax.set_ylim(args.ymin, args.ymax)

    ax.legend(loc='lower right')
    ax.set_xlabel(xlabel)
    ax.set_ylabel('CDF over traces')

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    if sv:
        fig.savefig(args.output[0:-4] + extra_sv_name + ".svg", bbox_inches='tight')
        print('Saved figure to', args.output[0:-4] + ".svg")

def plot_bar_data(args, data, coding_schemes, percentiles, extra, ylabel):
    multiplier = 1.0
    if "%" in ylabel:
        multiplier = 100.0
    fig, ax = plt.subplots()
    ax.set_box_aspect(1/2)

    width = 1
    start_pos = 0
    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        filtered_data[code] = []
        filtered_data[code +"_mean"] = np.mean(data[code])
        for p in percentiles:
            idx = int((p/100.0)*data[code].size)
            filtered_data[code].append(data[code][idx] * multiplier)
    with open (args.output[:-4] + extra  +
            "_data_bar_{}_{}.json".format(args.ymin, args.ymax), 'w') as f:
        json.dump(filtered_data, f)

    xticklabels = [f'{percent}th' for percent in percentiles]
    xticks = [7*i+2*width for i in range(len(percentiles))]
    xdata = 7 * np.arange(len(percentiles))
    for code in coding_schemes:
        ax.bar(xdata + start_pos,
            filtered_data[code],
            width=width,
            label=pretty_name[code],
            color=pretty_color[code],
            clip_on=True,
            linewidth=0.3,
            edgecolor='black',
            # linestyle=pretty_linestyle[code],
            hatch=pretty_hatch[code])
        start_pos += width

    # avoid repeated labels
    handles, labels = fig.gca().get_legend_handles_labels()
    by_label = dict(zip(labels, handles))

    # used for generating freeze._bar_0_1.svg
    ax.legend(by_label.values(), by_label.keys(), loc='upper left',
              bbox_to_anchor=(0, 1.13))
    if "median" not in ylabel:
        ax.set_yticks(list(range(6)))

    ax.set_xlabel("Percentile over videos", fontsize=20)
    ax.set_ylabel(ylabel, fontsize=20)

    ax.set_xticks(xticks)
    ax.tick_params(axis='x', length=0)
    ax.set_xticklabels(xticklabels)

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    # ax.spines['bottom'].set_visible(False)
    ax.spines['top'].set_visible(False)

    filestem = args.output[:-4] + extra  +  "_bar_{}_{}".format(args.ymin, args.ymax)
    fig.savefig(f'{filestem}.svg', bbox_inches='tight')
    fig.savefig(f'{filestem}.pdf', bbox_inches='tight')
    print(f'Saved figures to {filestem}.svg/pdf')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument('--FECOnly', default=True, help='only non-recoverable frames when FEC used')
    parser.add_argument('--ymin', default=0, type=float,
                        help='min Y-axis value to plot')
    parser.add_argument('--ymax', default=1, type=float,
                        help='max Y-axis value to plot')
    parser.add_argument('--xmax', default = -1, type=float, help='X-axis value to clip to')
    parser.add_argument("--skip", required=False, action='append', default=[])
    args = parser.parse_args()

    with open(args.config, 'r') as f:
      d = json.load(f)
    args.config = os.path.basename(args.config)
    assert(len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])
    percentiles = [25, 50, 75, 90]
    extra_sv_names = ["", "_durations", "_average_durations", "_median_durations"]
    log_paths = []
    log_exists = True
    for name in extra_sv_names:
        log_paths.append(args.output[0:-4] + name + ".json")
        log_exists = log_exists and os.path.exists(log_paths[-1])
    if log_exists:
        with open(log_paths[0], 'r') as f:
          all_freezes = json.load(f)
        coding_schemes = all_freezes["coding_schemes"]
        del all_freezes["coding_schemes"]
        with open(log_paths[1], 'r') as f:
          all_freeze_durations = json.load(f)
        with open(log_paths[2], 'r') as f:
          all_average_freeze_durations = json.load(f)
        with open(log_paths[3], 'r') as f:
          all_median_freeze_durations = json.load(f)
    else:
        all_freezes, all_freeze_durations, all_average_freeze_durations, all_median_freeze_durations, coding_schemes = get_cdf_data(args, tau)
        conf = copy.deepcopy(all_freezes)
        conf["coding_schemes"] = coding_schemes
        with open(log_paths[0], 'w') as f:
            json.dump(conf, f)
        with open(log_paths[1], 'w') as f:
            json.dump(all_freeze_durations, f)
        with open(log_paths[2], 'w') as f:
            json.dump(all_average_freeze_durations, f)
        with open(log_paths[3], 'w') as f:
            json.dump(all_median_freeze_durations, f)
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

    # workaround: adjust the order of schemes to plot
    coding_schemes = ['ReedSolomonMultiFrame_0', 'ReedSolomonMultiFrame_3',
                      'StreamingCode_3-point_9', 'StreamingCode_3', 'StreamingCode_3-high-BW']
    del_point = False
    for item in args.skip:
        if "point" in item:
            del_point = True
    if del_point:
        del coding_schemes[2]
    plot_data(args, all_freezes, coding_schemes, extra_sv_names[0], "Frequency of freezes (%)", False, False)
    plot_data(args, all_freeze_durations, coding_schemes, extra_sv_names[1], "Cumulative duration of freezes (%)", False, False)
    plot_bar_data(args, all_freezes, coding_schemes, percentiles, extra_sv_names[0], "Frequency of freezes (%)")
    plot_bar_data(args, all_freeze_durations, coding_schemes, percentiles, extra_sv_names[1], "Percent of video spent frozen")

    clipped = ""
    if args.xmax > 0:
        clipped = " clipped to {}".format(args.xmax)
    plot_data(args, all_median_freeze_durations, coding_schemes, extra_sv_names[3], "Median duration of freezes (ms)" + clipped, True, False)
    plot_bar_data(args, all_median_freeze_durations, coding_schemes, percentiles, extra_sv_names[3], "Median duration of freezes (ms)")

#    plot_data(args, all_average_freeze_durations, coding_schemes, extra_sv_names[2], "Average freeze duration (ms)" + clipped, True)
#    plot_bar_data(args, all_average_freeze_durations, coding_schemes, percentiles, extra_sv_names[2], "Average freeze duration (ms)")


if __name__ == '__main__':
    main()
    