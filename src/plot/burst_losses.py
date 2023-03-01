#!/usr/bin/env python3

from cmath import log
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
                          pretty_linestyle, baseline,pretty_hatch,
                          is_proper_log_dir)

plt.rcParams['font.size'] = 20
plt.rcParams['legend.fontsize'] = 20

def get_plot_data(args):
    coding_schemes = []
    plot_data = {}

    for fname in os.scandir(args.log):
        fsplit = fname.name.split('.')

        if is_proper_log_dir(fname, args.config):
            coding_scheme = ''.join(fsplit[2:])
            if coding_scheme in args.skip:
                continue
            coding_schemes.append(coding_scheme)

            plot_data[coding_scheme] = {}
            for burst_len in range(1, args.max_burst + 1):
                plot_data[coding_scheme][burst_len] = [0] * (burst_len + 1)
            plot_data[coding_scheme][args.max_burst+1] = []

            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue

                try:
                    burst_indices = json.load(open(os.path.join(trace_log.path, "receiver", "burst_idx")))
                    frame_loss = json.load(open(os.path.join(trace_log.path, "receiver", "received_frames_loss_info_after_fec")))["frame_losses"]

                    for bs, be in zip(burst_indices["burst_start"], burst_indices["burst_end"]):
                        num_recovered = 0
                        burst_len = be-bs+1

                        for idx in range(bs, be+1):
                            if not frame_loss[idx]:
                                num_recovered += 1

                        if burst_len > args.max_burst:
                            plot_data[coding_scheme][args.max_burst+1].append((num_recovered, burst_len))
                        else:
                            plot_data[coding_scheme][burst_len][num_recovered] += 1
                except:
                    pass

    return plot_data, coding_schemes

def plot_bar(args, plot_data, coding_schemes):
    fig, ax = plt.subplots()
    ax.set_box_aspect(1/1.4)

    xticks = []
    xticklabels = []
    width = 0.28 * 4.0 / 5

    if args.mean:
        mean_non_recovered_lengths = {}
        start_pos = 0
        for code in coding_schemes:
            mean_non_recovered_lengths[code] = []
            for blen in range(1, args.max_burst+1):
                total_received = 0.0
                total_lost = 0.0
                for i in range(blen+1):
                    total_received += i*plot_data[code][blen][i]
                    total_lost += blen * plot_data[code][blen][i]
                if total_lost == 0.0:
                    mean = 0.0
                else:
                    mean = (total_lost - total_received) / total_lost
                mean_non_recovered_lengths[code].append(mean)
            if len(plot_data[code][args.max_burst+1]) > 0:
                extra = 1
                total_non_recovered = sum([1.0 - item[0] / item[1] for item in plot_data[code][args.max_burst+1]])
                mean_non_recovered = total_non_recovered / len(plot_data[code][args.max_burst+1])
                mean_non_recovered_lengths[code].append(mean_non_recovered)
            else:
                extra = 0

            # Set position of bar on X axis
            x_data = np.arange(1, args.max_burst+1 + extra)

            ax.bar(x_data+start_pos,
                mean_non_recovered_lengths[code],
                width=width,
                label=pretty_name[code],
                color=pretty_color[code],
                clip_on=False,
                linewidth=0.3,
                edgecolor='black',
                hatch=pretty_hatch[code])

            start_pos += width

        ax.set_xlabel('Burst length in number of lost frames', fontsize=23)
        ax.set_ylabel('Mean non-recovered frames\nin each burst', fontsize=23)
        multiplier = 1.
        if len(list(args.skip)) == 1:
            multiplier -= width / 4
        ax.set_xticks([item + multiplier*width for item in range(1, args.max_burst+2) ])
        xticklabels = [str(i) for i in range(1, args.max_burst+2)]
        xticklabels[-1] += "+"
        ax.set_xticklabels(xticklabels)
        ax.legend(loc='upper left')
        with open(args.output[0:-4] + "_data.json", 'w') as f:
            json.dump(mean_non_recovered_lengths, f)
    else:
        for code in coding_schemes:
            start_pos = 0

            for burst_len in range(1, args.max_burst + 1):
                x_data = np.arange(burst_len + 1)
                if np.sum(plot_data[code][burst_len]) == 0:
                    y_data = 0
                else:
                    y_data = (100 * np.array(plot_data[code][burst_len])
                        / np.sum(plot_data[code][burst_len]))

                ax.bar(x_data + start_pos,
                    y_data,
                    width=width,
                    label=pretty_name[code],
                    color=pretty_color[code],
                    alpha=0.7,
                    clip_on=False)

                xticks += range(start_pos, start_pos + burst_len + 1)
                xticklabels += [str(i) for i in range(burst_len + 1)]
                start_pos += burst_len + 3

        # avoid repeated labels
        handles, labels = fig.gca().get_legend_handles_labels()
        by_label = dict(zip(labels, handles))
        ax.legend(by_label.values(), by_label.keys(),
                loc='upper right')

        ax.set_xlabel('frames recovered from a burst loss of', color='black', fontsize=16)
        ax.set_ylabel('Percent')

        ax.set_xticks(xticks)
        ax.tick_params(axis='x', length=0)
        ax.set_xticklabels(xticklabels)

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)

    fig.savefig(args.output[:-4] +".svg", bbox_inches='tight')
    print('Saved figure to', args.output[:-4] +".svg")
    fig.savefig(args.output[:-4]+".pdf", bbox_inches='tight')
    print('Saved figure to', args.output[:-4]+".pdf")



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument("--skip", required=False, action='append', default=[])
    parser.add_argument('--offset', required=True, type=float,
                        help='offset of the text below X-axis')
    parser.add_argument('--max_burst', required=False, type=int, default=4,
                        help='Maximum length of burst to plot')
    parser.add_argument('--mean', required=False, action="store_true",
                        help='Plot just the mean')

    args = parser.parse_args()
    args.skip = [x.strip() for x in args.skip]  # workaround

    args.config = os.path.basename(args.config)
    log_path = args.output[0:-4]
    log_exists = os.path.exists(log_path)
    if log_exists:
        with open(log_path, 'r') as f:
            coding_schemes = json.load(f)["coding_schemes"]
        plot_data = {}
        for code in coding_schemes:
            with open(log_path + code +".json", 'r') as f:
                d = json.load(f)
            plot_data[code] = {}
            for key in d.keys():
                plot_data[code][int(key)] = d[key]
    else:
        plot_data, coding_schemes = get_plot_data(args)
        conf = copy.deepcopy(plot_data)
        for key in conf.keys():
            with open(log_path + key +".json", 'w') as f:
                json.dump(conf[key], f)
        with open(log_path, 'w') as f:
            json.dump({"coding_schemes" : coding_schemes}, f)

    if args.skip == ["StreamingCode_3"]:
        coding_schemes = [item for item in coding_schemes if item != "StreamingCode_3-high-BW"] + ["StreamingCode_3"]
        data = plot_data["StreamingCode_3-high-BW"]
        plot_data["StreamingCode_3"] = data
        del plot_data["StreamingCode_3-high-BW"]
        print("Hack to present Tambur-full_BW as Tambur. Must add in caption that Tambur here has the full BW")
    elif len(list(args.skip)) == 2 and "StreamingCode_3" in args.skip and "StreamingCode_3-point_9" in args.skip:
        coding_schemes = [item for item in coding_schemes if (item != "StreamingCode_3-high-BW") and  (item != "StreamingCode_3-point_9")] + ["StreamingCode_3"]
        data = plot_data["StreamingCode_3-high-BW"]
        plot_data["StreamingCode_3"] = data
        del plot_data["StreamingCode_3-high-BW"]
        if "StreamingCode_3-point_9" in plot_data:
            del plot_data["StreamingCode_3-point_9"]
        print("Hack to present Tambur-full_BW as Tambur. Must add in caption that Tambur here has the full BW")
    else:
        for coding_scheme in coding_schemes:
            if coding_scheme in args.skip and coding_scheme in plot_data:
                del plot_data[coding_scheme]
        coding_schemes = [item for item in coding_schemes if item not in args.skip]

    # workaround to adjust the order of schemes to plot
    coding_schemes = ['ReedSolomonMultiFrame_0', 'ReedSolomonMultiFrame_3', 'StreamingCode_3']
    plot_bar(args, plot_data, coding_schemes)

if __name__ == '__main__':
    main()
