#!/usr/bin/env python3

import os
import sys
import copy
import argparse
import json
import numpy as np
import pandas as pd
from pathlib import Path

import argparse
import json
import numpy as np

from plot_common import (plt, pretty_name, pretty_color,
                          pretty_linestyle, baseline,
                          get_trace_encode, cdf_data_helper,
                          is_proper_log_dir, get_trace_decode)

plt.rcParams['font.size'] = 20
plt.rcParams['axes.labelsize'] = 20


def get_box_data(args, tau):
    encode = {}
    decode = {}
    coding_schemes = []

    for fname in os.scandir(args.log):
        fsplit = fname.name.split('.')
        if is_proper_log_dir(fname, args.config):
            coding_scheme = coding_scheme = ''.join(fsplit[2:])
            if coding_scheme in args.skip:
                continue
            use_tau = tau
            if (coding_scheme == baseline) or (coding_scheme == pretty_name[baseline]) or (pretty_name[coding_scheme] == pretty_name[baseline]):
              use_tau = 0
            coding_schemes.append(coding_scheme)
            encode[coding_scheme] = []
            decode[coding_scheme] = []
            for trace_log in os.scandir(fname):
                if not trace_log.is_dir():
                    continue
                te = get_trace_encode(trace_log, use_tau)
                td = get_trace_decode(trace_log)
                encode[coding_scheme].extend(te)
                decode[coding_scheme].extend(td)
            encode[coding_scheme] = [ a/1000 for a in encode[coding_scheme]]
            decode[coding_scheme] = [ a/1000 for a in decode[coding_scheme]]
    return encode, decode, coding_schemes

def plot_data(args, encode_data, decode_data, input_coding_schemes):
    coding_schemes = [scheme for scheme in input_coding_schemes if scheme not in args.skip]

    fig, ax = plt.subplots()
    fig.set_size_inches(4,3)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    encode_box_data = []
    decode_box_data = []
    for coding_scheme in coding_schemes:
        encode_box_data.append(encode_data[coding_scheme])
        decode_box_data.append(decode_data[coding_scheme])

    labels = [ pretty_name[coding_scheme] for coding_scheme in coding_schemes]

    c = 'C0'
    box = ax.boxplot(encode_box_data, vert=True, patch_artist=True, labels=labels, showfliers=False,
            boxprops=dict(facecolor=c, color=c),
            capprops=dict(color=c),
            whiskerprops=dict(color=c),
            flierprops=dict(color=c, markeredgecolor=c),
            medianprops=dict(color='C5'))
    plt.ylabel("Time to encode in ms")
    #fig.set_size_inches(4,3)
    fig.savefig(args.encode_output[:-4] + ".svg", bbox_inches='tight')
    print('Saved figure to', args.encode_output[:-4] + ".svg")
    fig.savefig(args.encode_output[:-4] + ".pdf", bbox_inches='tight')
    print('Saved figure to', args.encode_output[:-4] + ".pdf")

    fig, ax = plt.subplots()
    ax.set_box_aspect(1/1.5)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)

    box = ax.boxplot(decode_box_data, vert=True, patch_artist=True, labels=labels, showfliers=False,
            boxprops=dict(facecolor=c, color=c),
            capprops=dict(color=c),
            whiskerprops=dict(color=c),
            flierprops=dict(color=c, markeredgecolor=c),
            medianprops=dict(color='C5'))
    plt.ylabel("Time to decode in ms")
    fig.set_size_inches(4,3)
    fig.savefig(args.decode_output[:-4] + ".svg", bbox_inches='tight')
    print('Saved figure to', args.decode_output[:-4] + ".svg")
    fig.savefig(args.decode_output[:-4] + ".pdf", bbox_inches='tight')
    print('Saved figure to', args.decode_output[:-4] + ".pdf")



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--log', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config name')
    parser.add_argument('--encode_output', required=True, help='output encode figure')
    parser.add_argument('--decode_output', required=True, help='output decode figure')
    parser.add_argument("--skip", required=False, action='append', default=[])

    args = parser.parse_args()
    args.config = os.path.basename(args.config)

    with open(args.config, 'r') as f:
      d = json.load(f)
    assert(len(d["StreamingCode"]["taus"]) == 1 or d["StreamingCode"]["taus"] == [d["StreamingCode"]["taus"][0]]*len(d["StreamingCode"]["taus"]))
    tau = int(d["StreamingCode"]["taus"][0])

    log_path_encode = args.encode_output[0:-4] + "_encode.json"
    log_path_decode = args.decode_output[0:-4] + "_decode.json"
    log_path_median_encode = args.encode_output[0:-4] + "_median_encode.json"
    log_path_median_decode = args.decode_output[0:-4] + "_median_decode.json"

    log_exists = os.path.exists(log_path_encode) and os.path.exists(log_path_decode)
    if log_exists:
        with open(log_path_encode, 'r') as f:
          encode_data = json.load(f)
        coding_schemes = encode_data["coding_schemes"]
        del encode_data["coding_schemes"]
        with open(log_path_decode, 'r') as f:
          decode_data = json.load(f)
        coding_schemes = decode_data["coding_schemes"]
        del decode_data["coding_schemes"]
    else:
        encode_data, decode_data, coding_schemes = get_box_data(args, tau)
        conf_encode = copy.deepcopy(encode_data)
        conf_encode["coding_schemes"] = coding_schemes
        with open(log_path_encode, 'w') as f:
            json.dump(conf_encode, f)
        conf_decode = copy.deepcopy(decode_data)
        conf_decode["coding_schemes"] = coding_schemes
        with open(log_path_decode, 'w') as f:
            json.dump(conf_decode, f)
    encode_box_data = []
    decode_box_data = []
    encode_times = {}
    decode_times = {}
    for coding_scheme in coding_schemes:
        encode_box_data.append(encode_data[coding_scheme])
        decode_box_data.append(decode_data[coding_scheme])
        encode_times[coding_scheme] = np.median(np.array([item for item in encode_box_data[-1]]))
        decode_times[coding_scheme] = np.median(np.array([item for item in decode_box_data[-1]]))
    with open(log_path_median_encode, 'w') as f:
        json.dump(encode_times, f)
    with open(log_path_median_decode, 'w') as f:
        json.dump(decode_times, f)

    plot_data(args, encode_data, decode_data, coding_schemes)

if __name__ == '__main__':
    main()
    