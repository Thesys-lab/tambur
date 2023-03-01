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
                          get_recover_delay_in_frames,
                          is_proper_log_dir)

plt.rcParams['font.size'] = 18
plt.rcParams['legend.fontsize'] = 18

def get_plot_data(args):
    coding_schemes = []
    plot_data = {}
    all_data = {}

    for fname in os.scandir(args.log):
        fsplit = fname.name.split('.')

        if is_proper_log_dir(fname, args.config):
            coding_scheme = ''.join(fsplit[2:])
            if coding_scheme in args.skip:
                continue
            coding_schemes.append(coding_scheme)

            plot_data[coding_scheme] = [0]*(args.max_frames + 2)
            all_data[coding_scheme] = []
            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue

                try:
                  all_data[coding_scheme] += get_recover_delay_in_frames(trace_log, args.max_frames, fec_only = True, recovered_only = True)
                except:
                    pass
    for coding_scheme in coding_schemes:
      total = max(1.0, float(len(all_data[coding_scheme])))
      for latency in all_data[coding_scheme]:
        if (latency == -1):
          plot_data[coding_scheme][args.max_frames + 1] += 1
        else:
          plot_data[coding_scheme][latency] += 1
      plot_data[coding_scheme] = [100.0 * float(item) / total for item in plot_data[coding_scheme]]

    return plot_data, coding_schemes

def plot_bar(args, plot_data, coding_schemes):
  fig, ax = plt.subplots()
  fig.set_size_inches(8, 3.5)

  xticks = []
  xticklabels = []
  width = 0.3 * 4.0 / 5
  start_pos = 0
  for code in coding_schemes:
    # Set position of bar on X axis
    x_data = 2*np.arange(0, args.max_frames + 2)
    ax.bar(x_data+start_pos,
        plot_data[code],
        width=width,
        label=pretty_name[code],
        color=pretty_color[code],
        clip_on=False,
        linewidth=0.3,
        edgecolor='black',
        hatch=pretty_hatch[code])

    start_pos += width

  ax.set_xticks([2*item + 2*width for item in range(0, args.max_frames+2)])
  xticklabels = [str(i) for i in range(0, args.max_frames+1)] + ["Unrecovered"]
  ax.set_xticklabels(xticklabels)
  # avoid repeated labels
  handles, labels = fig.gca().get_legend_handles_labels()
  by_label = dict(zip(labels, handles))
  ax.legend(by_label.values(), by_label.keys(),
          loc='upper right')

  ax.set_xlabel('# of extra frames used to recover a frame', fontsize=22)
  ax.set_ylabel('Percent of frames', fontsize=22)

  # hide the right and top spines
  ax.spines['right'].set_visible(False)

  ax.spines['top'].set_visible(False)
  fig.savefig(args.output[:-4] + ".svg", bbox_inches='tight')
  print('Saved figure to', args.output[:-4] + ".svg")
  fig.savefig(args.output[:-4] + ".pdf", bbox_inches='tight')
  print('Saved figure to', args.output[:-4] + ".pdf")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--output', required=True, help='output figure')
    parser.add_argument("--skip", required=False, action='append', default=[])
    parser.add_argument("--max_frames", type=int, required=False, default=3)

    args = parser.parse_args()
    args.config = os.path.basename(args.config)

    log_path = args.output[0:-4] + ".json"
    log_exists = os.path.exists(log_path)
    if log_exists:
      with open(log_path, 'r') as f:
          plot_data = json.load(f)
      coding_schemes = plot_data["coding_schemes"]
      del plot_data["coding_schemes"]
    else:
      plot_data, coding_schemes = get_plot_data(args)
      conf = copy.deepcopy(plot_data)
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

    # workaround to adjust the order of schemes to plot
    reordered_coding_schemes = []
    for item in ['ReedSolomonMultiFrame_0', 'ReedSolomonMultiFrame_3',
                      'StreamingCode_3-point_9', 'StreamingCode_3', 'StreamingCode_3-high-BW']:
      if item in coding_schemes:
        reordered_coding_schemes.append(item)
      elif item == 'StreamingCode_3-high-BW' and 'StreamingCode_3_high-BW' in coding_schemes:
        reordered_coding_schemes.append('StreamingCode_3_high-BW')
    plot_bar(args, plot_data, reordered_coding_schemes)


if __name__ == '__main__':
    main()
