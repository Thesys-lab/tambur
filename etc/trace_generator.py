from pathlib import Path as Path
import random
import json
def obtainTraceVideo(traceFname):
  """
  Obtains information about the trace
  ARGUMENTS
  ---------
      (str) traceFname: file name of trace to be parsed
  RETURNS
  -------
      (list/int) trace: list of packet receptions (0) and losses (1)
      (list/int) received_ttimes: list of packet transmitted times
      (list/bool) packet_parities: list of boolean values 1 for parity 0 for data
      (list/int) packet_sizes: list of sizes of packets
      (list/int) frame_numbers: list of frame numbers for packets
  """
  with open(Path(traceFname +".json"), 'r') as f:
    d = json.load(f)

  return d["trace"], d["received_ttimes"], d["packet_parities"], \
         d["packet_sizes"], d["frame_numbers"]

def generateTraceVideo(traceFname, length, p_loss = 0.5, min_num_data_pkts = 1, \
    max_num_data_pkts = 10, min_num_parity_pkts = 0, max_num_parity_pkts = 5, \
    min_pkt_size = 1000, max_pkt_size = 1000):
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
    d["trace"] += [random.uniform(0,1) < p_loss for _ in range(num_pkts)]
    d["received_ttimes"] += [i for i in range(time, time + num_pkts)]
    time += 1
    d["packet_parities"] += [0]*num_data_pkts + [1]*num_parity_pkts
    d["packet_sizes"] += [random.randint(min_pkt_size, max_pkt_size)] * num_pkts
    d["frame_numbers"] += [frame_num] * num_pkts
    frame_num += 1
  with open(Path(traceFname +".json"), 'w') as f:
    json.dump(d, f)

fname = "trace-3"
length = 1000
generateTraceVideo(fname, length, 0.035)
print(obtainTraceVideo(fname))
