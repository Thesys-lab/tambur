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
                          pretty_linestyle, pretty_hatch, cdf_data_helper, baseline,
                          get_non_recoverable_frames,
                          get_non_decoded_video_frames, is_proper_log_dir)


plt.rcParams['font.size'] = 18
plt.rcParams['legend.fontsize'] = 16


def get_cdf_data(args, tau, decode_type = 0, key_frames_only = False):
    undecoded_frames = {}
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
            non_recoverable_frames_percents = []
            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue
                if (decode_type == 0):
                  nrf = get_non_recoverable_frames(trace_log, use_tau, args.FECOnly)
                else:
                  nrf = get_non_decoded_video_frames(trace_log, use_tau, args.FECOnly, decode_type == 1, key_frames_only)
                if nrf != -1:
                    non_recoverable_frames_percents.append(nrf)
            non_recoverable_frames_percents.sort()
            undecoded_frames[coding_scheme] = np.asarray(non_recoverable_frames_percents)
    return undecoded_frames, coding_schemes

def plot_data(args, cdf_data, coding_schemes, extra_save=""):
    fig, ax = plt.subplots()
    ax.set_box_aspect(1/1.5)

    # only plot data points within the desired Y-axis range
    filtered_data = {}
    for code in coding_schemes:
        filtered_data[code] = [[], []]
        data = []
        for x, y in zip(cdf_data[code][0], cdf_data[code][1]):
            if y >= args.ymin and y <= args.ymax:
                filtered_data[code][0].append(x)
                filtered_data[code][1].append(y)
            data.append(y)


    for code in coding_schemes:
        ax.plot(filtered_data[code][0],
                filtered_data[code][1],
                label=pretty_name[code],
                color=pretty_color[code],
                linestyle=pretty_linestyle[code],
                linewidth=2,
                clip_on=False)

    ax.set_ylim(args.ymin, args.ymax)

    ax.legend(loc='lower right')
    ax.set_xlabel('Non-recoverable frames per trace (%)')
    ax.set_ylabel('CDF')

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)

    fig.savefig(args.output[0:-4] + extra_save + "_{}_{}.svg".format(args.ymin, args.ymax), bbox_inches='tight')
    print('Saved figure to', args.output[0:-4] + extra_save + "_{}_{}.svg".format(args.ymin, args.ymax))
    fig.savefig(args.output[0:-4] + extra_save + "_{}_{}.pdf".format(args.ymin, args.ymax), bbox_inches='tight')
    print('Saved figure to', args.output[0:-4] + extra_save + "_{}_{}.pdf".format(args.ymin, args.ymax))


def plot_bar_data(args, data, coding_schemes, percentiles, extra_save=""):
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
    with open (args.output[:-4]  + extra_save +
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

    ax.set_xlabel('Percentile over videos', fontsize=22)
    if "display" in extra_save or "render" in extra_save:
        ax.set_ylabel('Non-rendered frames (%)', fontsize=22)
    else:
        ax.set_ylabel('Non-recoverable frames (%)', fontsize=22)

    ax.set_xticks(xticks)
    ax.tick_params(axis='x', length=0)
    ax.set_xticklabels(xticklabels)

    # ax.text(-1, args.offset, '1 frame')
    # ax.text(3.25, args.offset, '2 frames')
    # ax.text(8.75, args.offset, '3 frames')
    # ax.text(15.25, args.offset, '4 frames')

    # hide the right and top spines
    ax.spines['right'].set_visible(False)
    # ax.spines['bottom'].set_visible(False)

    ax.spines['top'].set_visible(False)

    filestem = args.output[:-4]  + extra_save + "_bar_{}_{}".format(args.ymin, args.ymax)
    fig.savefig(f'{filestem}.svg', bbox_inches='tight')
    fig.savefig(f'{filestem}.pdf', bbox_inches='tight')
    print(f'Saved figures to {filestem}.svg/pdf')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument('--FECOnly', default=True, help='only non-recoverable frames when FEC used')
    parser.add_argument('--no_video', default=False, help='if True FEC used without VC')

    parser.add_argument('--ymin', default=0, type=float,
                        help='min Y-axis value to plot')
    parser.add_argument('--ymax', default=1, type=float,
                        help='max Y-axis value to plot')
    parser.add_argument("--skip", required=False, action='append', default=[])
    percentiles = [25, 50, 75, 90]
    args = parser.parse_args()
    with open(args.config, 'r') as f:
      d = json.load(f)
    assert(len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])
    args.config = os.path.basename(args.config)

    log_path_0 = args.output[0:-4] + "_0.json"
    log_exists_0 = os.path.exists(log_path_0)
    if log_exists_0:
        with open(log_path_0, 'r') as f:
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
        with open(log_path_0, 'w') as f:
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
    # workaround to adjust the order of schemes to plot
    del_point = False
    for item in args.skip:
        if "point" in item:
            del_point = True
    reordered_coding_schemes = []
    for item in ['ReedSolomonMultiFrame_0',  'ReedSolomonMultiFrame_3', 'StreamingCode_3-point_9', 'StreamingCode_3', 'StreamingCode_3-high-BW']:
        if item in coding_schemes:
            if not (item == 'StreamingCode_3-point_9' and del_point):
                continue
            reordered_coding_schemes.append(item)
        elif item == 'StreamingCode_3-high-BW' and 'StreamingCode_3_high-BW' in coding_schemes:
            reordered_coding_schemes.append('StreamingCode_3_high-BW')
    coding_schemes = reordered_coding_schemes
    if args.no_video:
        plot_data(args, cdf_data_helper(cdf_data), coding_schemes, "")
        plot_bar_data(args, cdf_data, coding_schemes, percentiles, "")

    log_path_1 = args.output[0:-4] + "_1.json"
    log_exists_1 = os.path.exists(log_path_1)
    if log_exists_1:
        with open(log_path_1, 'r') as f:
            cdf_data = json.load(f)
        coding_schemes = cdf_data["coding_schemes"]
        del cdf_data["coding_schemes"]
        for key in cdf_data.keys():
            cdf_data[key] = np.asarray(cdf_data[key])
    else:
        cdf_data, coding_schemes = get_cdf_data(args, tau, 1)
        conf = copy.deepcopy(cdf_data)
        for key in conf.keys():
            conf[key] = conf[key].tolist()
        conf["coding_schemes"] = coding_schemes
        with open(log_path_1, 'w') as f:
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

    # workaround to adjust the order of schemes to plot
    del_point = False
    for item in args.skip:
        if "point" in item:
            del_point = True
    reordered_coding_schemes = []
    for item in ['ReedSolomonMultiFrame_0',  'ReedSolomonMultiFrame_3', 'StreamingCode_3-point_9', 'StreamingCode_3', 'StreamingCode_3-high-BW']:
        if item in coding_schemes:
            if item == 'StreamingCode_3-point_9' and del_point:
                continue
            reordered_coding_schemes.append(item)
        elif item == 'StreamingCode_3-high-BW' and 'StreamingCode_3_high-BW' in coding_schemes:
            reordered_coding_schemes.append('StreamingCode_3_high-BW')
    coding_schemes = reordered_coding_schemes
    if not args.no_video:
        plot_data(args, cdf_data_helper(cdf_data), coding_schemes, "_displayed")
        plot_bar_data(args, cdf_data, coding_schemes, percentiles,  "_displayed")

    log_path_2 = args.output[0:-4] + "_2.json"
    log_exists_2 = os.path.exists(log_path_2)
    if log_exists_2:
        with open(log_path_2, 'r') as f:
            cdf_data = json.load(f)
        coding_schemes = cdf_data["coding_schemes"]
        del cdf_data["coding_schemes"]
        for key in cdf_data.keys():
            cdf_data[key] = np.asarray(cdf_data[key])
    else:
        cdf_data, coding_schemes = get_cdf_data(args, tau, 2)
        conf = copy.deepcopy(cdf_data)
        for key in conf.keys():
            conf[key] = conf[key].tolist()
        conf["coding_schemes"] = coding_schemes
        with open(log_path_2, 'w') as f:
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
    if not args.no_video:
        plot_data(args, cdf_data_helper(cdf_data), coding_schemes, "_video")
        plot_bar_data(args, cdf_data, coding_schemes, percentiles,  "_video")

    log_path_1_t = args.output[0:-4] + "_1_t.json"
    log_exists_1_t = os.path.exists(log_path_1_t)
    if log_exists_1_t:
        with open(log_path_1_t, 'r') as f:
            cdf_data = json.load(f)
        coding_schemes = cdf_data["coding_schemes"]
        del cdf_data["coding_schemes"]
        for key in cdf_data.keys():
            cdf_data[key] = np.asarray(cdf_data[key])
    else:
        cdf_data, coding_schemes = get_cdf_data(args, tau, 1, True)
        conf = copy.deepcopy(cdf_data)
        for key in conf.keys():
            conf[key] = conf[key].tolist()
        conf["coding_schemes"] = coding_schemes
        with open(log_path_1_t, 'w') as f:
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

    log_path_2_t = args.output[0:-4] + "_2_t.json"
    log_exists_2_t = os.path.exists(log_path_2_t)
    if log_exists_2_t:
        with open(log_path_2_t, 'r') as f:
            cdf_data = json.load(f)
        coding_schemes = cdf_data["coding_schemes"]
        del cdf_data["coding_schemes"]
        for key in cdf_data.keys():
            cdf_data[key] = np.asarray(cdf_data[key])
    else:
        cdf_data, coding_schemes = get_cdf_data(args, tau, 2, True)
        conf = copy.deepcopy(cdf_data)
        for key in conf.keys():
            conf[key] = conf[key].tolist()
        conf["coding_schemes"] = coding_schemes
        with open(log_path_2_t, 'w') as f:
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

if __name__ == '__main__':
    main()
