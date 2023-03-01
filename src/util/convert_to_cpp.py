#!/usr/bin/env python3
import torch
import math
import random
import json
import sys
from os import listdir
from pathlib import Path
from PredictParameters import PredictParameters
from DataLoader import convert_indicators_into_input_vector_all_indicators
from ML_Module import get_neural_network,neural_network_args,NUM_USE_INDICATORS,MODEL,get_sensitivity_pre_args
from Indicators import compute_multi_frame_sufficient_guardspace

def convert_to_string(vals):
  s = ""
  for val in vals:
    s += str(val) + "!"
  return s

def save_NN(sensitivity, save_file_name):
  sensitivity_args = get_sensitivity_pre_args(sensitivity)
  larger_model = get_neural_network(*neural_network_args(*sensitivity_args))
  model = larger_model.predictive_model
  means = larger_model.means
  variances = larger_model.variances
  example = torch.rand(1,39)
  torch_script_module = torch.jit.trace(model, example)
  torch_script_module.save(save_file_name)
  epsilon = 0.00001
  with open(save_file_name[:-3] + "_means.txt", 'w') as f:
    f.write(convert_to_string(means))

  with open(save_file_name[:-3] + "_vars.txt", 'w') as f:
    f.write(convert_to_string(variances))

  if (sensitivity == 4):
    input_s = ""
    converted_s = ""
    output_s = ""
    for _ in range(10000):
      next_input = [round(float(random.uniform(0,1)),3) for _ in range(39)]
      converted = []
      for j in range(13):
        converted += [float((next_input[13*z + j] - means[3*j + z])/ math.sqrt(variances[3*j + z] + epsilon)) for z in range(3)]
      next_input_converted = torch.tensor([converted])
      for z in range(39):
        input_s += str(next_input[z]) + "!"
        converted_s += str(next_input_converted[0][z].item()) + "!"
      next_output = model(next_input_converted)
      output_s += str(next_output[0][0].item()) + "!" + str(next_output[0][1].item()) + "!"
    with open(save_file_name[:-3] + "_test_inputs.txt", 'w') as f:
      f.write(input_s)

    with open(save_file_name[:-3] + "_test_converted_inputs.txt", 'w') as f:
      f.write(converted_s)

    with open(save_file_name[:-3] + "_test_outputs.txt", 'w') as f:
      f.write(output_s)

if __name__ == "__main__":
  sensitivities = [0,1,2,4,5]
  labels = ["Tambur-low-BW", "Tambur0.5", "Tambur0.9", "Tambur", "Tambur-high-BW"]
  save_file_names = ["jit_NNs/" + item +".pt" for item in labels]
  for (sensitivity, save_fname) in zip(sensitivities, save_file_names):
    save_NN(sensitivity, save_fname)
