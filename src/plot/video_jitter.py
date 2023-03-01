#!/usr/bin/env python3

import os
import sys
import copy
import argparse
import json
import numpy as np
import pandas as pd

import argparse
import json
import numpy as np

from plot_common import (plt, pretty_name, pretty_color,
                          pretty_linestyle, cdf_data_helper, baseline,
                          is_proper_log_dir,
                          get_latency_video_frames,
                          is_proper_log_dir)


def get_cdf_data(args, tau, consume):
    jitter = {}
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

            jitter[coding_scheme] = []

            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue
                latencies = get_latency_video_frames(trace_log, use_tau, True, consume, False)
                if (len(latencies) > 0):
                    jitter[coding_scheme].append(np.var(latencies))
            jitter[coding_scheme].sort()
    return jitter, coding_schemes

def plot_data(args, jitter, coding_schemes, extra=""):

    fig, ax = plt.subplots()
    ignore = []
    for code in coding_schemes:
        jitter[code] = np.asarray(jitter[code])
        if jitter[code].size == 0:
            del jitter[code]
            ignore.append(code)
    cdf_data = cdf_data_helper(jitter)

    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        if code in ignore:
            continue
        filtered_data[code] = [[], []]
        for x, y in zip(cdf_data[code][0], cdf_data[code][1]):
            if y >= args.ymin and y <= args.ymax:
                filtered_data[code][0].append(x)
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
    ax.set_xlabel(f'''Jitter''')
    ax.set_ylabel('CDF')

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)


    fig.savefig(args.output[0:-4] + extra +".svg", bbox_inches='tight')
    print('Saved figure to', args.output[0:-4] + extra +".svg")

def plot_bar_data(args, data, coding_schemes, percentiles, extra_save=""):
    fig, ax = plt.subplots()
    width = 0.2
    start_pos = 0
    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        filtered_data[code] = []
        filtered_data[code +"_mean"] = np.mean(data[code])
        for p in percentiles:
            idx = int((p/100.0)*data[code].size)
            filtered_data[code].append(data[code][idx])
    with open (args.output[:-4]  + extra_save +
            "_data_bar_{}_{}.json".format(args.ymin, args.ymax), 'w') as f:
        json.dump(filtered_data, f)

    xticklabels = [str(percent) for percent in percentiles]
    xticks = [i+1.5*width for i in range(len(percentiles))]
    xdata = np.arange(len(percentiles))
    for code in coding_schemes:
        ax.bar(xdata + start_pos,
            filtered_data[code],
            width=width,
            label=pretty_name[code],
            color=pretty_color[code],
            alpha=0.7,
            clip_on=False,
            linestyle=pretty_linestyle[code])
        start_pos += width

    # avoid repeated labels
    handles, labels = fig.gca().get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(),
            loc='upper right')

    ax.set_xlabel('Percentile over videos')
    ax.set_ylabel('Jitter')

    ax.set_xticks(xticks)
    ax.tick_params(axis='x', length=0)
    ax.set_xticklabels(xticklabels)

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['bottom'].set_visible(False)

    ax.spines['top'].set_visible(False)
    fig.savefig(args.output[:-4]  + extra_save + "_bar_{}_{}.svg".format(args.ymin, args.ymax), bbox_inches='tight')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument('--ymin', default=0, type=float,
                        help='min Y-axis value to plot')
    parser.add_argument('--ymax', default=1, type=float,
                        help='max Y-axis value to plot')
    parser.add_argument("--skip", required=False, action='append', default=[])
    args = parser.parse_args()
    percentiles = [25, 50, 75, 90]
    args.config = os.path.basename(args.config)
    with open(args.config, 'r') as f:
      d = json.load(f)
    assert(len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])

    log_path_consume = args.output[0:-4] + "_consume.json"
    log_exists_consume = os.path.exists(log_path_consume)
    if log_exists_consume:
        with open(log_path_consume, 'r') as f:
            decode_time = json.load(f)
        coding_schemes = decode_time["coding_schemes"]
        del decode_time["coding_schemes"]
    else:
        decode_time, coding_schemes = get_cdf_data(args, tau, True)
        conf = copy.deepcopy(decode_time)
        for key in conf.keys():
            conf[key] = conf[key]
        conf["coding_schemes"] = coding_schemes
        with open(log_path_consume, 'w') as f:
            json.dump(conf, f)
    plot_data(args, decode_time, coding_schemes, "_consume")
    plot_bar_data(args, decode_time, coding_schemes, percentiles, "_consume")
    log_path_decode = args.output[0:-4] + "_decode.json"
    log_exists_decode = os.path.exists(log_path_decode)
    if log_exists_decode:
        with open(log_path_decode, 'r') as f:
            decode_time = json.load(f)
        coding_schemes = decode_time["coding_schemes"]
        del decode_time["coding_schemes"]
    else:
        decode_time, coding_schemes = get_cdf_data(args, tau, False)
        conf = copy.deepcopy(decode_time)
        for key in conf.keys():
            conf[key] = conf[key]
        conf["coding_schemes"] = coding_schemes
        with open(log_path_decode, 'w') as f:
            json.dump(conf, f)
    plot_data(args, decode_time, coding_schemes, "_decode")
    plot_bar_data(args, decode_time, coding_schemes, percentiles, "_decode")

if __name__ == '__main__':
    main()