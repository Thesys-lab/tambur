#!/usr/bin/env python3

import os
import argparse
import json
import copy
import time

import sys
import signal
import string
import subprocess
import random
import csv
import numpy as np

# helper functions


def print_cmd(cmd):
    if type(cmd) == list:
        cmd_str = ' '.join(cmd).strip()
    elif type(cmd) == str:
        cmd_str = cmd.strip()

    print('$', cmd_str)


def run(cmd, **kwargs):
    print_cmd(cmd)
    return subprocess.run(cmd, **kwargs)


def Popen(cmd, **kwargs):
    print_cmd(cmd)
    return subprocess.Popen(cmd, **kwargs)

# check if IP forwarding is enabled


def check_ip_forwarding():
    ret = subprocess.check_output(['sysctl', 'net.ipv4.ip_forward'])

    if int(ret.decode().strip().split('=')[-1]) != 1:
        sys.exit('ERROR: Please run "sudo sysctl -w net.ipv4.ip_forward=1" '
                 'to enable IP forwarding')


# read the width and height of a .y4m video
def read_resolution(y4m_path):
    MAX_BUFFER = 1024

    width = None
    height = None

    with open(y4m_path, 'r') as y4m_fh:
        try:
            line = y4m_fh.readline(MAX_BUFFER)
        except:
            sys.exit(f'ERROR: {y4m_path} is not a valid .y4m file')

        for item in line.split():
            if item[0] == 'W':
                width = int(item[1:])
            elif item[0] == 'H':
                height = int(item[1:])

    if width and height:
        return (width, height)
    else:
        sys.exit(f'ERROR: {y4m_path} is not a valid .y4m file')

# wait for sender to output "Waiting for receiver..."


def wait_for_sender_ready(sender_proc):
    HINT = "Waiting for receiver..."

    while True:
        if not (sender_proc and sender_proc.poll() is None):
            sys.exit('ERROR: sender is gone')

        line = sender_proc.stderr.readline()
        if not line:
            sys.exit('ERROR: sender did not output to stderr?')

        if HINT in line:
            # server is ready
            break


def tear_down(proc):
    if proc:
        # force kill everything in the process group
        os.killpg(os.getpgid(proc.pid), signal.SIGKILL)


def get_ge(conf, pos):
    if conf["fix_ge"]:
        return conf["t_gb"], conf["t_bg"], conf["l_g"], conf["l_b"], pos
    else:
        random.seed(pos + conf["seed"])
        if conf["random_variable_ge"]:
            t_gb = random.uniform(conf["random_variable_ge_gb"][0], conf["random_variable_ge_gb"][1])
            t_bg = random.uniform(conf["random_variable_ge_bg"][0], conf["random_variable_ge_bg"][1])
            l_g = random.uniform(conf["random_variable_ge_l_g"][0], conf["random_variable_ge_l_g"][1])
            l_b = random.uniform(conf["random_variable_ge_l_b"][0], conf["random_variable_ge_l_b"][1])
            vals = [t_gb, t_bg, l_g, l_b]
        else:
            with open(conf["variable_ge"], 'r') as f:
                variable_conf = list(csv.reader(f, delimiter=","))
            vals = variable_conf[random.randrange(0, len(variable_conf) - 1)]
        return *[float(vals[i]) for i in range(4)], pos

if __name__ == '__main__':
    use_mahimahi = True
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', required=True, help='config file')
    args = parser.parse_args()
    with open(args.config, 'r') as f:
        conf = json.load(f)
    sender_port = conf["sender_port"]
    receiver_port = conf["port"]
    check_ip_forwarding()
    if "fps" not in conf:
        conf["fps"] = 30
    if "cbrs" not in conf:
        conf["cbrs"] = [500]
    if "lazy" not in conf:
        conf["lazy"] = 2 # other options are 1 and 0
    host = "$MAHIMAHI_BASE" if use_mahimahi else '127.0.0.1'
    sender_host = "$MAHIMAHI_BASE" if use_mahimahi else '127.0.0.1'
    i = 0
    with open(os.path.join(conf["video_folder"], "videos.json"), 'r') as f:
        video_names = json.load(f)["videos"]
    for cbr in conf["cbrs"]:
        for video_num, video in enumerate(video_names):
            # verify input .y4m video and read its width and height
            width, height = read_resolution(video)
            conf["video_name"] = os.path.basename(video + "_{}".format(cbr))
            ge = get_ge(conf, i)
            i += 1
            conf["t_gb"] = ge[0]
            conf["t_bg"] = ge[1]
            conf["l_g"] = ge[2]
            conf["l_b"] = ge[3]
            conf["width"] = width
            conf["height"] = height
            for coding_scheme in conf["coding_schemes"]:
                assert (len(conf[coding_scheme]["taus"]) ==
                        len(conf[coding_scheme]["models"]))
                assert (len(conf[coding_scheme]["taus"]) ==
                        len(conf[coding_scheme]["model_means"]))
                assert (len(conf[coding_scheme]["taus"]) ==
                        len(conf[coding_scheme]["model_vars"]))
                for pos in range(len(conf[coding_scheme]["taus"])):
                    tmp_conf = copy.deepcopy(conf)
                    tmp_conf["coding_scheme"] = coding_scheme
                    tau = conf[coding_scheme]["taus"][pos]
                    model = conf[coding_scheme]["models"][pos]
                    model_means = conf[coding_scheme]["model_means"][pos]
                    model_vars = conf[coding_scheme]["model_vars"][pos]
                    tmp_conf[coding_scheme]["model"] = model
                    tmp_conf[coding_scheme]["model_mean"] = model_means
                    tmp_conf[coding_scheme]["model_var"] = model_vars
                    tmp_conf["seed"] = conf["seed"] + ge[4]
                    tmp_conf["tau"] = conf[coding_scheme]["taus"][pos]
                    if (not os.path.exists(conf["experiment_log_folder"])):
                        os.mkdir(conf["experiment_log_folder"])
                    log_folder = conf["experiment_log_folder"] + "/" + \
                        os.path.basename(args.config) + "." + coding_scheme + \
                        "_{}".format(tau)
                    if (coding_scheme == "StreamingCode"):
                        if "high-BW" in model:
                            log_folder += "-high-BW"
                        if "point_9" in model:
                            log_folder += "-point_9"
                    if (not os.path.exists(log_folder)):
                        os.mkdir(log_folder)
                    log_folder += "/" + coding_scheme + \
                        "_" + conf["video_name"] + "/"
                    if (not os.path.exists(log_folder)):
                        os.mkdir(log_folder)
                    sender_log_file = log_folder + "sender_video.csv"
                    receiver_log_file = log_folder + "receiver_video"
                    use_conf = args.config + "." + \
                        coding_scheme + "_{}".format(tau)
                    if (coding_scheme == "StreamingCode" and "high-BW" in model):
                        use_conf += "-high-BW"
                    if (coding_scheme == "StreamingCode" and "point_9" in model):
                        use_conf += "-point_9"
                    use_conf += "_tmp_{}_{}".format(video_num, cbr)
                    with open(use_conf, "w") as f:
                        json.dump(tmp_conf, f)
                    command_path = os.path.join(os.path.dirname(os.getcwd()), "tambur/src/ringmaster_app")
                    sender_cmd = os.path.join(command_path,"video_sender") +" --video " \
                        + video + " -v -o " + sender_log_file  \
                        + " --config " + use_conf + " "  # + " --host " + host
                    receiver_cmd = os.path.join(command_path,"video_receiver") +" --fps {} --cbr {} --lazy {} -v -o ".format(conf["fps"], cbr,conf["lazy"]) \
                        + receiver_log_file + " --config " + use_conf + " " + " --sender_host " + \
                        sender_host + " --host " + host

                    # start video_sender as a subprocess and wait for it to be ready
                    global sender_proc
                    sender_proc = Popen(sender_cmd, shell=True,text=True, #stderr=subprocess.PIPE, 
                                        preexec_fn=os.setsid)
                    time.sleep(1)
#                    wait_for_sender_ready(sender_proc)
                    mm = "mm-delay {}".format(conf["one_way_delay"])
                    if use_mahimahi:
                        receiver_cmd = f"{mm} sh -c '{receiver_cmd}'"

                    # start video_receiver as another subprocess
                    global receiver_proc
                    receiver_proc = Popen(receiver_cmd, shell=True, text=True,
                                          preexec_fn=os.setsid)
                    print('Receiver is up')

                    # run experiment for 'timeout + 5' seconds
                    time.sleep(conf["timeout"] + 5)
                    print('Experiment is done')
                    # tear down everything
                    tear_down(sender_proc)
                    tear_down(receiver_proc)
