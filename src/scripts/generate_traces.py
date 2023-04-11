from pathlib import Path as Path
import os
import random
import json
import argparse

def generateTraceVideo(traceFname, length, p_loss = 0.5, min_num_data_pkts = 1, \
    max_num_data_pkts = 8, min_num_parity_pkts = 0, max_num_parity_pkts = 5, \
    min_pkt_size = 256, max_pkt_size = 2000):
  """
  Generates a dummy trace
  ARGUMENTS
  ---------
      (str) traceFname: file name of trace to be parsed
      (int) length: number of frames per trace
      (float) p_loss: probability a packet is lost, where losses are i.i.d.
      (int) min_num_data_pkts: minimum number of data packets for frame
      (int) max_num_data_pkts: maximum number of data packets for frame
      (int) min_num_parity_pkts: minimum number of parity packets for frame
      (int) max_num_parity_pkts: maximum number of parity packets for frame
      (int) min_pkt_size: minimum size of data packets of frame in bytes
      (int) max_pkt_size: maximum size of data packets of frame in bytes
  RETURNS
  -------
  """
  assert(p_loss >= 0 and p_loss < 1)
  assert (min_num_data_pkts > 0 and max_num_data_pkts >= min_num_data_pkts)
  assert (min_num_parity_pkts >= 0 and max_num_parity_pkts >= min_num_parity_pkts)
  assert(min_pkt_size > 0 and max_pkt_size >= min_pkt_size)
  d = { "trace": [], "received_ttimes": [], "packet_parities": [], \
      "packet_sizes": [], "frame_numbers": [] }
  time = 0
  frame_num = 0
  for _ in range(length):
    num_data_pkts = random.randint(min_num_data_pkts, max_num_data_pkts)
    num_parity_pkts = random.randint(min_num_parity_pkts, max_num_parity_pkts)
    num_pkts = num_data_pkts + num_parity_pkts
    d["trace"] += [random.uniform(0,1) < p_loss for _ in range(num_pkts)] # 1 if and only if the packet is lost
    d["received_ttimes"] += [i for i in range(time, time + num_pkts)] # Dummy values for transmit times
    time += 1
    d["packet_parities"] += [0]*num_data_pkts + [1]*num_parity_pkts # 0's for data packets and 1's for parity packets
    d["packet_sizes"] += [random.randint(min_pkt_size, max_pkt_size)] * num_pkts # sizes of packets in bytes
    d["frame_numbers"] += [frame_num] * num_pkts # label packets of frame with corresponding frame number
    frame_num += 1
  with open(Path(traceFname +".json"), 'w') as f:
    json.dump(d, f)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--outputFolder', required=True, help='Folder to save traces')
    parser.add_argument("--num_traces", type=int, required=False, default=5)
    parser.add_argument("--length", type=int, required=False, default=1000)
    parser.add_argument("--p_loss", type=int, required=False, default=.05)
    args = parser.parse_args()
    os.mkdir(args.outputFolder)
    for i in range(args.num_traces):
      generateTraceVideo(os.path.join(args.outputFolder, str(i)), args.length,args.p_loss)

