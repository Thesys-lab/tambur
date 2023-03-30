from pathlib import Path as Path
import os
from subprocess import Popen
import argparse

def main(output, traces_folder, config):
  commands = []
  for trace in os.listdir(traces_folder):
    command_file = os.path.join(os.path.dirname(os.getcwd()), "tambur/src/fec_examples/get_ge_params ")
    command = command_file + config + " " + os.path.join(traces_folder, trace) + " " + output + " ; "
    commands.append(command)
  for command in commands:
    try:
        p = Popen(command, shell=True)
        p.wait()
    except:
        continue
    print("\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', required=True, help='File name to save info on GE params')
    parser.add_argument("--traces_folder", required=True, help='Folder containing traces')
    parser.add_argument("--config", required=True, help='Config file for computing GE parameters')
    args = parser.parse_args()
    main(args.output, args.traces_folder, args.config)