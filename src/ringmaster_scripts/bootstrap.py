#!/usr/bin/env python3

import os
from subprocess import Popen
import random
import shutil
import argparse
import json
import time
import multiprocessing

def main(video_folder, configs, num_sender_receiver_pairs, offset):
  files = list(os.listdir(video_folder))
  random.shuffle(files)
  assert num_sender_receiver_pairs > 0
  if (num_sender_receiver_pairs > len(files)):
    num_sender_receiver_pairs = len(files)
  num_sender_receiver_pairs = int(num_sender_receiver_pairs // len(configs)) * int(len(configs))
  cores_per_config = num_sender_receiver_pairs // len(configs)
  assert num_sender_receiver_pairs > 0
  tmp ="tmp_folder_{}".format(offset) 
  if (os.path.exists(tmp)):
     shutil.rmtree(tmp)
  os.mkdir(tmp)
  all_traces = []
  commands = []
  videos = {}
  for core_pair in range(cores_per_config):
    videos[core_pair] = []
    traces = os.path.join(tmp, "traces_{}".format(core_pair))
    all_traces.append(traces)
    os.mkdir(traces)
  for i in range(len(files)):
    tf = os.path.join(video_folder, files[i])
    videos[i % cores_per_config].append(tf)
  for i in range(cores_per_config):
    with open(os.path.join(all_traces[i], "videos.json"), 'w') as f:
        json.dump({"videos": videos[i]}, f)
  for i, config in enumerate(configs):
    for core_pair in range(cores_per_config):
      with open(config, 'r') as f:
        conf = json.load(f)
      conf["video_folder"] = all_traces[core_pair]
      conf["experiment_log_folder"] = os.path.join(tmp, "logs_{}_{}".format(i,core_pair))
      if (not os.path.exists(conf["experiment_log_folder"])):
        os.mkdir(conf["experiment_log_folder"])
      conf["port"] = 2*(cores_per_config * i + core_pair) + 49152 + offset
      conf["sender_port"] = conf["port"] + 1
      conf["seed"] = core_pair * len(files)
      if "lazy" not in conf:
        conf["lazy"] = 2
      assert(conf["lazy"] in [0,1,2]) # see ``--lazy <level>'' in ``src/ringmaster_app/video_receiver.cc''
      conf_name = os.path.join(tmp, "config_{}_{}".format(i, core_pair))
      command_file = os.path.join(os.path.dirname(os.getcwd()), "tambur/src/ringmaster_scripts/run-config.py")
      commands.append(
            "python3 " + command_file + " --config=" +
            conf_name+ " ;")
      with open(conf_name, 'w') as f:
        json.dump(conf,f)
  procs = []
  for command in commands:
    procs.append(Popen(command, shell=True))
    time.sleep(2)
  for proc in procs:
    proc.wait()
    time.sleep(2)
  if (os.path.exists(conf["log_folder"])):
      shutil.rmtree(conf["log_folder"])
  for i, config in enumerate(configs):
    for core_pair in range(cores_per_config):
      with open(config, 'r') as f:
        conf = json.load(f)
      if (not os.path.exists(conf["log_folder"])):
        os.mkdir(conf["log_folder"])
        shutil.copyfile(config,os.path.join(conf["log_folder"], os.path.basename(config)))
      mv_logs = os.path.join(tmp, "logs_{}_{}".format(i,core_pair))
      print("mv_logs", mv_logs)
      for mv_log in os.listdir(mv_logs):
        print("mv_log", mv_log)
        if ("." not in mv_log):
            continue
        mv_log_name = str(os.path.basename(config)) + "." + mv_log[mv_log.index(".")+1:]
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
#  shutil.rmtree(tmp)
#  for fname in os.listdir():
#    if (fname[:len("tmp_configs.")] == "tmp_configs."):
#      shutil.rmtree(fname)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--videos_folder', required=True, help='videos folder location')
    parser.add_argument('--config', required=True, help='Config file', action='append')
    parser.add_argument('--num_sender_receiver_pairs', required=False, default = 1, type=int, help='number of experiments to trun in parallel')
    parser.add_argument('--offset', required=False, default = 0, type=int, help='Setting for --offset for the port')
    args = parser.parse_args()
    num_configs = len(args.config)
    num_sender_receiver_pairs = int(args.num_sender_receiver_pairs // num_configs) * int(num_configs)
    main(args.videos_folder, args.config, args.num_sender_receiver_pairs, args.offset)
    print("Set correct max length video in video_sender")
