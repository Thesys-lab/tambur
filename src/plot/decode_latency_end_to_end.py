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

from plot_common import (plt, pretty_name, pretty_color, pretty_hatch,
                        pretty_linestyle, cdf_data_helper, baseline,
                         is_proper_log_dir,
                         get_latency_video_frames,
                         is_proper_log_dir, latency_of_not_recovered)

plt.rcParams['font.size'] = 18
plt.rcParams['legend.fontsize'] = 16


def get_cdf_data(args, tau, consume, all_decodes):
    decode_time = {}
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

            decode_time[coding_scheme] = []

            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue
                latencies = get_latency_video_frames(trace_log, use_tau, True, consume, all_decodes)
                if (len(latencies) > 0):
                    if all_decodes:
                        decode_time[coding_scheme] += latencies
                    elif args.percentile:
                        lat = np.asarray(latencies)
                        sort = sorted(lat)
                        pidx = int((float(args.percentile)/100.0)*lat.size)
                        decode_time[coding_scheme].append(sort[pidx])
                    else:
                        decode_time[coding_scheme].append(np.mean(latencies))
            decode_time[coding_scheme].sort()
    return decode_time, coding_schemes

def plot_data(args, decode_time, coding_schemes, extra="", sv = False):
    fig, ax = plt.subplots()
    ignore = []
    for code in coding_schemes:
        decode_time[code] = np.asarray(decode_time[code])
        if decode_time[code].size == 0:
            del decode_time[code]
            ignore.append(code)
    cdf_data = cdf_data_helper(decode_time)

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

    ax.legend(loc='upper left')
    if "all_latencies" in extra:
        ax.set_xlabel("End-to-end latency or {} if not displayed".format(latency_of_not_recovered))
        ax.set_ylabel('CDF over frames')
    else:
        ax.set_xlabel(f'''End-to-end latency''')
        ax.set_ylabel('CDF over traces')

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    if sv:
        fig.savefig(args.output[0:-4] + extra + ".svg", bbox_inches='tight')
        print('Saved figure to', args.output[0:-4] + extra + ".svg")

def plot_bar_data(args, data, coding_schemes, percentiles, extra = ""):
    fig, ax = plt.subplots()
    ax.set_box_aspect(1/1.8)

    width = 1
    start_pos = 0
    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        filtered_data[code] = []
        filtered_data[code +"_mean"] = np.mean(data[code])
        for p in percentiles:
            idx = int((p/100.0)*data[code].size)
            filtered_data[code].append(data[code][idx])
    with open (args.output[:-4] + extra  +
            "_data_bar_{}_{}.json".format(args.ymin, args.ymax), 'w') as f:
        json.dump(filtered_data, f)

    xticklabels = [f'{percent}th' for percent in percentiles]
    xticks = [7*i+2*width for i in range(len(percentiles))]
    xdata = 7*np.arange(len(percentiles))
    for code in coding_schemes:
        ax.bar(xdata + start_pos,
            filtered_data[code],
            width=width,
            label=pretty_name[code],
            color=pretty_color[code],
            clip_on=False,
            linewidth=0.3,
            edgecolor='black',
            hatch=pretty_hatch[code])
        start_pos += width

    # avoid repeated labels
    handles, labels = fig.gca().get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys(),
              loc='upper left', bbox_to_anchor=(0.0, 1.1) )

    if "all_latencies" in extra:
        ax.set_xlabel('Percentile over video frames', fontsize=22)
    else:
        ax.set_xlabel('Percentile over videos', fontsize=22)
    ax.set_ylabel("End-to-end latency (ms)", fontsize=22)

    ax.set_xticks(xticks)
    ax.tick_params(axis='x', length=0)
    ax.set_xticklabels(xticklabels)

    ax.set_ylim(0, 210)

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    #ax.spines['bottom'].set_visible(False)

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
    parser.add_argument('--ymin', default=0, type=float,
                        help='min Y-axis value to plot')
    parser.add_argument('--ymax', default=1, type=float,
                        help='max Y-axis value to plot')
    parser.add_argument("--skip", required=False, action='append', default=[])
    parser.add_argument("--percentile", required=False,
                        help="Plot the given percentile. By default it will plot mean")
    args = parser.parse_args()
    args.config = os.path.basename(args.config)
    with open(args.config, 'r') as f:
        d = json.load(f)
    assert (len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [
            d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])
    percentiles = [25, 50, 75]
    log_path_t_f = args.output[0:-4] + "_t_f.json"
    log_exists_t_f = os.path.exists(log_path_t_f)
    if log_exists_t_f:
        with open(log_path_t_f, 'r') as f:
            decode_time = json.load(f)
        coding_schemes = decode_time["coding_schemes"]
        del decode_time["coding_schemes"]
    else:
        decode_time, coding_schemes = get_cdf_data(args, tau, True, False)
        conf = copy.deepcopy(decode_time)
        conf["coding_schemes"] = coding_schemes
        with open(log_path_t_f, 'w') as f:
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

#    plot_data(args, decode_time, coding_schemes, "_displayed")
#    plot_bar_data(args, decode_time, coding_schemes, percentiles, "_displayed")

    log_path_f_f = args.output[0:-4] + "_f_f.json"
    log_exists_f_f = os.path.exists(log_path_f_f)
    if log_exists_f_f:
        with open(log_path_f_f, 'r') as f:
            decode_time = json.load(f)
        coding_schemes = decode_time["coding_schemes"]
        del decode_time["coding_schemes"]
    else:
        decode_time, coding_schemes = get_cdf_data(args, tau, False, False)
        conf = copy.deepcopy(decode_time)
        conf["coding_schemes"] = coding_schemes
        with open(log_path_f_f, 'w') as f:
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

#    plot_data(args, decode_time, coding_schemes, "_decode")
#    plot_bar_data(args, decode_time, coding_schemes, percentiles, "_decode")

    log_path_t_t = args.output[0:-4] + "_t_t.json"
    log_exists_t_t = os.path.exists(log_path_t_t)
    if log_exists_t_t:
        with open(log_path_t_t, 'r') as f:
            decode_time = json.load(f)
        coding_schemes = decode_time["coding_schemes"]
        del decode_time["coding_schemes"]
    else:
        decode_time, coding_schemes = get_cdf_data(args, tau, True, True)
        conf = copy.deepcopy(decode_time)
        conf["coding_schemes"] = coding_schemes
        with open(log_path_t_t, 'w') as f:
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

    # workaround to adjust the order to schemes to plot
    coding_schemes = ['ReedSolomonMultiFrame_0', 'ReedSolomonMultiFrame_3',
                      'StreamingCode_3-point_9', 'StreamingCode_3', 'StreamingCode_3-high-BW']
    del_point = False
    for item in args.skip:
        if "point" in item:
            del_point = True
    if del_point:
        del coding_schemes[2]
    plot_data(args, decode_time, coding_schemes, "_displayed_all_latencies", False)
    plot_bar_data(args, decode_time, coding_schemes, percentiles, "_displayed_all_latencies")

    log_path_f_t = args.output[0:-4] + "_f_t.json"
    log_exists_f_t = os.path.exists(log_path_f_t)
    if log_exists_f_t:
        with open(log_path_f_t, 'r') as f:
            decode_time = json.load(f)
        coding_schemes = decode_time["coding_schemes"]
        del decode_time["coding_schemes"]
    else:
        decode_time, coding_schemes = get_cdf_data(args, tau, False, True)
        conf = copy.deepcopy(decode_time)
        for key in conf.keys():
            conf[key] = conf[key]
        conf["coding_schemes"] = coding_schemes
        with open(log_path_f_t, 'w') as f:
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

#    plot_data(args, decode_time, coding_schemes, "_decode_all_latencies")
#    plot_bar_data(args, decode_time, coding_schemes, percentiles, "_decode_all_latencies")

if __name__ == '__main__':
    main()