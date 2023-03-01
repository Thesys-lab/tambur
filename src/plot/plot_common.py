#!/usr/bin/env python3

from ast import Try
import os
import re
import sys
import argparse
import json
import numpy as np
import pandas as pd
from statistics import mean

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

baseline = "ReedSolomonMultiFrame_0"

latency_of_not_recovered = 300

plt.style.use('seaborn-deep')
plt.rcParams['font.size'] = 16
plt.rcParams['axes.labelsize'] = 16
plt.rcParams['legend.fontsize'] = 16
plt.rcParams['svg.fonttype'] = 'none'
plt.rcParams['font.family'] = 'Times New Roman'


pretty_name = {
    'baseline': 'Block-Within',
    "ReedSolomonMultiFrame_0" : "Block-Within",
    'ReedSolomonMultiFrame_3' : 'Block-Multi',
    "StreamingCode_3" : "Tambur",
    "StreamingCode_3-high-BW" : "Tambur-full-BW",
    "StreamingCode_3_high-BW" : "Tambur-full-BW",
    "StreamingCode_3_Tamburpt" : "Tambur",
    "StreamingCode_3_Tambur-low-BWpt" : "Tambur-low-BW",
    "StreamingCode_3_Tambur-high-BWpt" : "Tambur-full-BW",
    "StreamingCode_3-point_9" : "Tambur-0.9"

}

pretty_color = {
    'baseline': 'C0',
    "ReedSolomonMultiFrame_0" : "C0",
    'ReedSolomonMultiFrame_3' : 'C4',
    "StreamingCode_3" : "C2",
    "StreamingCode_3-high-BW" : "C3",
    "StreamingCode_3_high-BW" : "C3",
    "StreamingCode_3_Tamburpt" : "C2",
    "StreamingCode_3_Tambur-low-BWpt" : "C5",
    "StreamingCode_3_Tambur-high-BWpt" : "C5",
    "StreamingCode_3-point_9" : "#008080"
}

pretty_linestyle = {
    'baseline': '--',
    "ReedSolomonMultiFrame_0" : "--",
    'ReedSolomonMultiFrame_3' : ':',
    "StreamingCode_3" : "-",
    "StreamingCode_3-high-BW" : "-.",
    "StreamingCode_3_high-BW" : "-.",
    "StreamingCode_3_Tamburpt" : "-",
    "StreamingCode_3_Tambur-low-BWpt" : ":",
    "StreamingCode_3_Tambur-high-BWpt" : "-.",
    "StreamingCode_3-point_9" : (0, (3, 1, 1, 1))
}

pretty_hatch = {
    'baseline': '\\',
    "ReedSolomonMultiFrame_0" : 2*"\\",
    'ReedSolomonMultiFrame_3' : 2*'/',
    "StreamingCode_3" : "",
    "StreamingCode_3_Tamburpt" : "",
    "StreamingCode_3-high-BW" : 2*"x",
    "StreamingCode_3_high-BW" : 2*"x",
    "StreamingCode_3_Tambur-high-BWpt" : 2*"x",
    "StreamingCode_3-point_9" : 2*"."

}


'''
input: {'baseline': [val1, val2, ...], 'streaming': [val1, val2, ...], ...}
output: {'baseline': [[bin_edge, ...], [cumulative_num_val, ...]], 'streaming': [...], ...}
'''
def cdf_data_helper(input_data):
    # find min and max values among all codes
    min_val = None
    max_val = None

    for code in list(input_data.keys()):
        code_min = np.min(input_data[code])
        if min_val is None or code_min < min_val:
            min_val = code_min

        code_max = np.max(input_data[code])
        if max_val is None or code_max > max_val:
            max_val = code_max

    # prepare CDF data with the help of histogram
    cdf_data = {}
    cdf_data['min_val'] = min_val
    cdf_data['max_val'] = max_val
    bins = np.linspace(min_val, max_val, num=2000)

    for code in list(input_data.keys()):
        hist, bin_edges = np.histogram(input_data[code], bins)

        x = bin_edges
        y = np.cumsum(hist) / len(input_data[code])
        y = np.insert(y, 0, 0)  # prepend 0

        cdf_data[code] = [x, y]

    return cdf_data

def is_proper_log_dir(fname, config_name):
    fsplit = fname.name.split('.')
    return ((fsplit[0] + '.' + fsplit[1]) == config_name and fname.is_dir())

def fec_used(frame_size_df, tau, frame_num, pos = 2):
  include_frame = False
  for j in range(frame_num, min(len(frame_size_df[pos]), frame_num +tau+1)):
    if int(frame_size_df[pos][j]) > 0:
      include_frame = True
  return include_frame

def get_mean_bandwidth_overhead(trace_log, tau, fec_only = True):
    frame_size_f = os.path.join(trace_log.path, "sender", "frame_size")
    try:
        frame_size_df = pd.read_csv(frame_size_f, header=None)
        data_sum = 0
        par_sum = 0
        for i in range(frame_size_df.shape[0]):
          include_frame = (not fec_only) or fec_used(frame_size_df, tau, i)
          if include_frame:
            data_sum += int(frame_size_df[1][i])
            par_sum += int(frame_size_df[2][i])
        if data_sum > 0:
            bw_overhead = (100.0 * par_sum)/data_sum
        else:
            bw_overhead = 0
        return bw_overhead
    except:
        return 0

def get_non_decoded_video_frames(trace_log, tau, fec_only = True, consume = True, keyframes_only = False):
    fname = "receiver_video_"
    if consume:
        fname += "consume"
    else:
        fname += "decode"
    recover_frames_log_f = os.path.join(trace_log.path, fname +".csv")
    sent_f = os.path.join(trace_log.path, "sender_video.csv")
    try:
      sent_df = pd.read_csv(sent_f, header=None)
      receive_df = pd.read_csv(recover_frames_log_f, header=None)

      total_frames   = 0
      decoded_frames = 0
      decoded_ids = {}
      for i in range(receive_df.shape[0]):
          decoded_ids[receive_df[0][i]] = True
      for i in range(sent_df.shape[0]):
        include_frame = (not fec_only) or fec_used(sent_df, tau, i, 10)
        include_frame = include_frame and ((not keyframes_only) or (sent_df[5][i] == 1))
        if include_frame:
          total_frames += 1
          decoded_frames += i in decoded_ids
      if total_frames == 0:
          return -1
      return 100.0 - (100.0 * decoded_frames)/total_frames
    except:
      return -1

def get_times_between_decoding_failures(trace_log, tau, consume):
  fname = "receiver_video_"
  index = 2
  if consume:
    fname += "consume"
  else:
    fname += "decode"
    index = 1
  recover_frames_log_f = os.path.join(trace_log.path, fname +".csv")
  sent_f = os.path.join(trace_log.path, "sender_video.csv")
  try:
    sent_df = pd.read_csv(sent_f, header=None)
    receive_df = pd.read_csv(recover_frames_log_f, header=None)
    periods = []
    decoded = {}
    for i in range(receive_df.shape[0]):
      decoded[receive_df[0][i]] = receive_df[index][i]
    first_time = None
    last_time = None
    for i in range(sent_df.shape[0]):
      if i in decoded:
        if first_time is None:
          first_time = decoded[i] / 1000.0
        last_time = decoded[i] / 1000.0
      else:
        if (None not in [first_time, last_time]):
          periods.append(last_time-first_time)
        first_time = None
        last_time = None
    return periods
  except:
    return []

def get_freezes_durations(trace_log, tau, fec_only = True):
  fname = "receiver_video_consume"
  recover_frames_log_f = os.path.join(trace_log.path, fname +".csv")
  sent_f = os.path.join(trace_log.path, "sender_video.csv")
  try:
    decoded = {}
    sent_df = pd.read_csv(sent_f, header=None)
    receive_df = pd.read_csv(recover_frames_log_f, header=None)
    freeze_durations = []
    freezes = 0.0
    total_time = 0.0
    num_durations = 0.0
    durations = []
    for i in range(receive_df.shape[0]):
      decoded[receive_df[0][i]] = i
      if (i >= 1):
          durations.append((receive_df[2][i] - receive_df[2][i-1]) / 1000.0)
    fec_protected_frame_displayed = not fec_only
    for i in range(31):
        FEC_used = fec_used(sent_df, tau, i, 10)
        if (not fec_only) or (FEC_used and i in decoded):
            fec_protected_frame_displayed = True
        if (fec_only and not FEC_used):
            fec_protected_frame_displayed = False

    for i in range(31, sent_df.shape[0]):
      include_frame = (not fec_only) or fec_used(sent_df, tau, i, 10)
      if include_frame and fec_protected_frame_displayed:
        if i in decoded:
          pos = decoded[i]
          if pos >= 31:
            duration = durations[pos-1]
            num_durations += 1.0
            total_time += duration
            avg_frame_duration_ms = sum(durations[pos-31:pos-1]) / 30.0
            threshold = max(3*avg_frame_duration_ms, avg_frame_duration_ms + 150)
            freeze = duration > threshold
            if freeze:
              freeze_durations.append(duration)
              freezes += 1
      if not fec_only:
          fec_protected_frame_displayed = True
      elif (not include_frame):
          fec_protected_frame_displayed = False
      else:
          if i in decoded:
              fec_protected_frame_displayed = True

    total_freeze_time = sum(freeze_durations)
    if len(freeze_durations) == 0:
        return 0.0, 0.0, 0.0, 0.0
    return total_freeze_time / total_time, freezes / num_durations, np.mean(np.array(freeze_durations)), np.median(np.array(freeze_durations))
  except:
    return 0.0, 0.0, 0.0, 0.0

def get_waiting_times(trace_log, consume):
  fname = "receiver_video_"
  index = 2
  if consume:
    fname += "consume"
  else:
    fname += "decode"
    index = 1
  recover_frames_log_f = os.path.join(trace_log.path, fname +".csv")
  sent_f = os.path.join(trace_log.path, "sender_video.csv")
  try:
    sent_df = pd.read_csv(sent_f, header=None)
    receive_df = pd.read_csv(recover_frames_log_f, header=None)
    waiting_times = []
    decoded = {}
    for i in range(receive_df.shape[0]):
      decoded[receive_df[0][i]] = receive_df[index][i]
    last_time = None
    for i in range(sent_df.shape[0]):
      if i in decoded:
        if not last_time is None:
          waiting_times.append(max(0.0,decoded[i] / 1000.0 - last_time))
        last_time = decoded[i] / 1000.0
    return waiting_times
  except:
    return []

def get_latency_video_frames(trace_log, tau, fec_only, consume, all_latencies):
  fname = "receiver_video_"
  index = 2
  if consume:
    fname += "consume"
  else:
    fname += "decode"
    index = 1
  recover_frames_log_f = os.path.join(trace_log.path, fname +".csv")
  sent_f = os.path.join(trace_log.path, "sender_video.csv")
  try:
    sent_df = pd.read_csv(sent_f, header=None)
    receive_df = pd.read_csv(recover_frames_log_f, header=None)
    latencies = []
    decoded = {}
    for i in range(receive_df.shape[0]):
      decoded[receive_df[0][i]] = receive_df[index][i]
    for i in range(sent_df.shape[0]):
      include_frame = (not fec_only) or fec_used(sent_df, tau, i, 10)
      if include_frame:
        if i in decoded:
          latencies.append((decoded[i] - sent_df[3][i]) / 1000.0) # divide by 1000.0 to convert to ms
        else:
          if all_latencies:
            latencies.append(latency_of_not_recovered)
    return latencies
  except:
    return []


def get_non_recoverable_frames(trace_log, tau, fec_only = True):
    received_frames_log_f = os.path.join(trace_log.path, "receiver", "received_frames_log")
    frame_size_f = os.path.join(trace_log.path, "sender", "frame_size")
    try:
      frame_size_df = pd.read_csv(frame_size_f, header=None)
      received_frames_df = pd.read_csv(received_frames_log_f, header=None)

      total_frames   = 0
      decoded_frames = 0
      for i in range(min(frame_size_df.shape[0], received_frames_df.shape[0]-1)):
        include_frame = (not fec_only) or fec_used(frame_size_df, tau, i)
        if include_frame:
          total_frames += 1
          decoded_frames += int(received_frames_df[1][i+1]) == 0
      if total_frames == 0:
          return -1
      return 100.0 - (100.0 * decoded_frames)/total_frames
    except:
      return -1

def get_recover_delay_in_frames(trace_log, tau, fec_only = True, recovered_only = True):
  received_frames_log_f = os.path.join(trace_log.path,    "receiver", "received_frames_log")
  frame_size_f = os.path.join(trace_log.path, "sender", "frame_size")
  recoveries = []
  try:
    frame_size_df = pd.read_csv(frame_size_f, header=None)
    received_frames_df = pd.read_csv(received_frames_log_f, header=None)
    for i in range(received_frames_df.shape[0]-1):
      include_frame = ((not fec_only) or fec_used(frame_size_df, tau, i))
      is_recovered = (0 == int(received_frames_df[10][i+1])) or not recovered_only
      if include_frame and is_recovered:
        if 1 == int(received_frames_df[1][i+1]) or int(received_frames_df[2][i+1]) > tau:
          recoveries.append(-1)
        else:
          for j in range(tau + 1):
            if int(received_frames_df[2][i+1]) == j:
              recoveries.append(j)
  except:
    pass
  return recoveries

def get_trace_jitter(trace_log):
    try:
        received_frames_log_f = os.path.join(trace_log.path, "receiver", "received_frames_log")

        decode_ts = pd.read_csv(received_frames_log_f)['decode_ts_']
        decode_ts = decode_ts[decode_ts > 0]
        if decode_ts.size > 0:
            diff = decode_ts.diff()
            diff = diff.iloc[1:]
            if diff.size > 0:
                return diff.var()
        return -1
    except:
        return -1

def get_decode_time_by_frame(trace_log, max_frames):
    dtbf = {}
    for i in range(max_frames + 1):
            dtbf[i] = []
    try:
        received_frames_log_f = os.path.join(trace_log.path, "receiver", "received_frames_log")
        df = pd.read_csv(received_frames_log_f, dtype=int)

        df = df[df["not_decoded_"] == 0]

        for _, row in df.iterrows():
            if row["decoded_after_frames_"] <= max_frames:
                dtbf[row["decoded_after_frames_"]].append(row["decode_ts_"] - row["first_ts_"])
    except:
        pass

    return dtbf

def get_trace_encode(trace_log, tau):
    encode_f = os.path.join(trace_log.path, "sender", "encode")
    frame_size_f = os.path.join(trace_log.path, "sender", "frame_size")
    try:
        frame_size_df = pd.read_csv(frame_size_f, header=None)
        encode_df = pd.read_csv(encode_f, header=None)
        times = []
        for i in range(frame_size_df.shape[0]):
          include_frame = fec_used(frame_size_df, tau, i)
          if include_frame:
            times.append(encode_df[0][i + 2 * tau + 1])
        return times
    except:
        return []

def get_trace_decode(trace_log):
    decode_f = os.path.join(trace_log.path, "receiver", "decode_time")
    try:
        decode_df = pd.read_csv(decode_f, header=None)
        return list(decode_df[0])
    except:
        return []