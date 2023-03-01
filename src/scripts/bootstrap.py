#!/usr/bin/env python3

import os
from subprocess import Popen
import random
import shutil
import argparse
import json
import time

def get_command(config, extra_arg,):

  return "src/scripts/run-config.sh --config=" + config + " " + extra_arg + " ; "

def main(trace_folder, configs, num_core_pairs, extra_args):
  files = list(os.listdir(trace_folder))
  random.shuffle(files)
  assert num_core_pairs > 0
  if (num_core_pairs > len(files)):
    num_core_pairs = len(files)
  num_core_pairs = int(num_core_pairs // len(configs)) * int(len(configs))
  cores_per_config = num_core_pairs // len(configs)
  assert num_core_pairs > 0
  tmp ="tmp_folder"
  if (os.path.exists(tmp)):
    shutil.rmtree(tmp)

  os.mkdir(tmp)
  all_traces = []
  commands = []
  for core_pair in range(cores_per_config):
    traces = os.path.join(tmp, "traces_{}".format(core_pair))
    all_traces.append(traces)
    os.mkdir(traces)
  for i in range(len(files)):
    tf = os.path.join(trace_folder, files[i])
    tf2 = os.path.join(all_traces[i % cores_per_config], files[i])
    shutil.copyfile(tf,tf2)
  for i, config in enumerate(configs):
    for core_pair in range(cores_per_config):
      with open(config, 'r') as f:
        conf = json.load(f)
      conf["trace_folder"] = all_traces[core_pair]
      conf["log_folder"] = os.path.join(tmp, "logs_{}_{}".format(i,core_pair))
      conf["port"] = 8080 + 2*(cores_per_config * i + core_pair)
      conf["sender_port"] = conf["port"] + 1
      conf_name = os.path.join(tmp, "config_{}_{}".format(i, core_pair))
      commands.append(get_command(conf_name, extra_args[i]))
      with open(conf_name, 'w') as f:
        json.dump(conf,f)
  procs = []
  for command in commands:
    procs.append(Popen(command, shell=True))
    time.sleep(2)
  for proc in procs:
    proc.wait()
    time.sleep(2)
  for i, config in enumerate(configs):
    for core_pair in range(cores_per_config):
      with open(config, 'r') as f:
        conf = json.load(f)
      if (not os.path.exists(conf["log_folder"])):
        os.mkdir(conf["log_folder"])
        shutil.copyfile(config,os.path.join(conf["log_folder"], os.path.basename(config)))
      mv_logs = os.path.join(tmp, "logs_{}_{}".format(i,core_pair))
      for mv_log in os.listdir(mv_logs):
        mv_log_name = str(os.path.basename(config)) + "." + mv_log[mv_log.index(".")+1:]
        if mv_log_name[-1] == "_":
          mv_log_name = mv_log_name[:-1]
        dest_folder = os.path.join(conf["log_folder"], mv_log_name)
        if (not os.path.exists(dest_folder)):
          os.mkdir(dest_folder)
          shutil.copyfile(config, os.path.join(dest_folder, mv_log_name))
          with open(os.path.join(dest_folder, "GE_params"), 'w') as f:
            print("create GE_params folder")
        mv_log_path = os.path.join(mv_logs,mv_log)
        for log in os.listdir(mv_log_path):
          if (log == "GE_params"):
            with open(os.path.join(dest_folder, "GE_params"), 'a') as output_f:
              with open(os.path.join(mv_log_path, log), 'r') as f:
                for line in f.readlines():
                  output_f.write(line)
          elif (log != str(os.path.basename(mv_log_path))):
            tf = os.path.join(mv_log_path, log)
            tf2 = os.path.join(dest_folder, log)
            shutil.copytree(tf,tf2)
  shutil.rmtree(tmp)
  for fname in os.listdir():
    if (fname[:len("tmp_configs.")] == "tmp_configs."):
      shutil.rmtree(fname)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--trace_folder', required=True, help='log folder location')
    parser.add_argument('--config', required=True, help='Config file', action='append')
    parser.add_argument('--num_core_pairs', required=False, default = 1, type=int, help='number of pairs of cores')
    parser.add_argument('--hc_ge', required=True, action='append', type=int, help='Whether --hc-ge should be included for corresponding config')
    args = parser.parse_args()
    assert (len(args.config) == len(args.hc_ge))
    extra_args = ["" for _ in range(len(args.config))]
    for i in range(len(args.config)):
      assert((args.hc_ge[i]) in [0,1])
      if (args.hc_ge[i]):
        extra_args[i] += "--hc-ge "
      extra_args[i] += "--no-mahi "
    num_configs = len(args.config)
    num_core_pairs = int(args.num_core_pairs // num_configs) * int(num_configs)
    main(args.trace_folder, args.config, args.num_core_pairs, extra_args)
