import json
import os
from os import listdir
from pathlib import Path
import sys
#from loss_stats import getRFCBurstStats,packetToFrameRepresentation,multi_frame_loss_fraction

def obtainTraceVideo(traceDirectory,traceName):
    """
    Obtain the bitmap of losses from traceName inside traceDirectory along with transmitted times frame numbers QR values and parities
    ARGUMENTS
    ---------
        (str) traceDirectory: directory of trace to be parsed
        (str) traceName: name of trace to be parsed
    RETURNS
    -------
        (list/int) trace: list of packet receptions (0) and losses (1)
        (list/int) received_ttimes: list of packet transmitted times
        (list/int) frame_numbers: list of frame numbers for packets
        (list/int) qr_values: list of QR values for packets
        (list/bool) packet_parities: list of boolean values 1 for parity 0 for data
        (list/int) packet_sizes: list of sizes of packets
    """
    print(traceDirectory + "/" + traceName)
    with open(Path(traceDirectory +"/" + traceName), 'r') as f:
            L = f.readlines()
    L2 = [item.split(",") for item in L]
    trace = [int(item[4]) for item in L2]
    received_ttimes = [float(item[5]) for item in L2]
    received_rtimes = [float(item[6]) for item in L2]
    qr_values = [int(item[3]) for item in L2]
    packet_parities = [int(item[1] == "True" or item[1] == "1") for item in L2]
    last_ps = 0
    last_fn = 0
    packet_sizes = []
    frame_numbers = []
    for i in range(len(L)):
        if L2[i][0] != "None":
            last_ps = int(L2[i][0])
        packet_sizes.append(last_ps)
        if L2[i][2] != "None":
            last_fn = int(L2[i][2])
        frame_numbers.append(last_fn)

    return trace,received_ttimes,received_rtimes,frame_numbers,qr_values,packet_parities,packet_sizes


def num_bursts(trace):
    cnt = 0
    idx = 0
    inside_burst = False
    while idx < len(trace):
        if inside_burst:
            if not trace[idx]:
                inside_burst = False
        else:
            if trace[idx]:
                inside_burst = True
                cnt += 1
        idx += 1

    return cnt



trace_dir = sys.argv[1]
gen_trace_dir = sys.argv[2]

for trace_file in listdir(trace_dir):
    if not trace_file.endswith(".txt"):
        continue
    trace,received_ttimes,received_rtimes,frame_numbers,qr_values,packet_parities,packet_sizes = obtainTraceVideo(trace_dir, trace_file)

    json_data = { "trace": [], "received_ttimes": [], "packet_parities": [], "packet_sizes": [], "frame_numbers": [] }
    json_data["trace"] = [bool(item) for item in trace]
    json_data["received_ttimes"] = received_ttimes
    json_data["packet_parities"] = [int(item) for item in packet_parities]
    json_data["packet_sizes"] = packet_sizes
    json_data["frame_numbers"] = frame_numbers


    if num_bursts(trace) < 2:
        continue

    with open(os.path.join(gen_trace_dir, trace_file.replace(".txt", ".json")), 'w') as wf:
            json.dump(json_data, wf)


