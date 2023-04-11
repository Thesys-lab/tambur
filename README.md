# Tambur

Tambur is an open-source library for the FEC schemes in our paper published at NSDI '23 — [Tambur: Efficient loss recovery for videoconferencing via streaming
codes](https://www.usenix.org/conference/nsdi23/presentation/rudow). The library is meant to enable rapid prototyping of new FEC schemes for videoconferencing. To ensure the confidentiality of the dataset the ML models used in our paper were trianed on, the models are not released as part of this library. Within the library, Tambur defaults to Tambur-full-BW until a new ML model is used for Tambur. 
Tambur is integrated with a videoconferencing research platform called Ringmaster, available [here](https://github.com/microsoft/ringmaster), which was also open-sourced as part of the same paper. 


### Third-party libraries
The following third-party libraries should be placed in "third_party"
```
https://jerasure.org/jerasure/gf-complete (Revision 1.03.)
http://jerasure.org/jerasure/jerasure (Revision 2.0)
https://github.com/nlohmann/json (version 3.10.4)
https://download.pytorch.org/libtorch/lts/1.8/cpu/libtorch-cxx11-abi-shared-with-deps-1.8.2%2Bcpu.zip 
https://github.com/gerddie/maxflow (version 3.04)
https://github.com/microsoft/ringmaster (Version 53c04ee)
```

### Dependencies
```
sudo apt-get install build-essential autoconf automake libtool g++ pkg-config googletest protobuf-compiler libprotobuf-dev autotools-dev dh-autoreconf iptables pkg-config dnsmasq-base  apache2-bin debhelper libssl-dev ssl-cert libxcb-present-dev libcairo2-dev libpango1.0-dev apache2-dev libgtest-dev -y ; 
sudo apt-get install libgtest-dev libz-dev jq libjerasure2; 
sudo apt install python3-pip ; 
pip3 install torch torchvision torchaudio pandas matplotlib ;
sudo add-apt-repository ppa:keithw/mahimahi ; 
sudo apt-get update ; sudo apt-get install mahimahi ; 
sudo apt install autoconf libvpx-dev libsdl2-dev ;
```

### Install script
```
./tambur_install.sh ;
```


### Example 1: UDP echo server and client

```
cd src/example/
./udp_echo_server <server_port>
./udp_echo_client <server_ip> <server_port>
```

### Example 2: UDP datagram serialization

```
cd src/example/
./udp_receiver <receiver_port>
./udp_sender <receiver_ip> <receiver_port>
```


You may need to run sudo sysctl -w net.ipv4.ip_forward=1 to run Mahimahi on your system (everytime it reboots)

# Example use with dummy sizes for video frames 
```
sudo sysctl -w net.ipv4.ip_forward=1 ;
python src/scripts/generate_traces.py --outputFolder OUTPUT_FOLDER --num_traces NUM_TRACES --length NUM_FRAMES ;
python src/scripts/bootstrap.py --num_sender_receiver_pairs <NUM_EXPERIMENTS_TO_RUN_IN_PARALLEL> --trace_folder <OUTPUT_FOLDER> --config configs/example_bootstrap.json --hc_ge 1; 
src/plot/generate_all_plots_FEC_only.sh --config=configs/example_bootstrap.json --plot-folder=<FOLDER_TO_SAVE_PLOTS_TO> ; 
```

# Tambur plus ringmaster example use
```
sudo sysctl -w net.ipv4.ip_forward=1 ;
python3 src/ringmaster_scripts/bootstrap.py --num_sender_receiver_pairs <NUM_EXPERIMENTS_TO_RUN_IN_PARALLEL> --videos_folder <FOLDER_CONTAINING_VIDEOS> --config ringmaster_configs/experiment_random_variable.json --offset <OFFSET_FOR_PORT>;
src/plot/generate_all_plots.sh --config=ringmaster_configs/experiment_random_variable.json --plot-folder=<FOLDER_TO_SAVE_PLOTS_TO>
```

## License
```
Copyright 2023, Carnegie Mellon University

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## Support
This work was funded in part by the National Science Foundation (NSF) under grant CCF-1910813.

### Scripts Description and Usage
```
# this script generates dummy traces used for fec_examples
src/scripts/generate_traces.py log_folder_name

sudo sysctl -w net.ipv4.ip_forward=1
# This script is used to get GE model parameters based on a config and a trace
# It returns transition_good_bad transition_bad_good loss_good_state loss_bad_state
```
src/fec_examples/get_ge_params  <config> <trace> 
```
# This script accepts a folder containing configs and runs for all configs
# optional arguments --hc-ge to use ge parameters from the config t_gb probability transition good to bad t_bg probability of transition bad to good l_g probability of loss in good state l_b probability of loss in bad state
# --no-mahi to turn off mahimahi
```
src/scripts/run-all-configs.sh --config_folder=<path>
```

# This script accepts a config and runs experiment according to that config 
# (It goes over all traces in the trace file)
```
src/scripts/run-config.sh --config=<path>
```
# It first runs get_ge_params over the trace and gets GE model parameters for MahiMahi
# It then starts multi_receiver on the port specified in the config
# Then, it calls sender_trace which sends data packets over MahiMahi
# using Packet sizes from the trace

#this script generates plots
```
src/plot/generate_all_plots.sh --config=<path> --plot-folder=<path>
```

### Some information about logging
* log_event: This function is used to note the timestamp(microseconds since epoch) of an event along with some additional information
* log_value: This function is used to note some value along with some additional information

Note all times are in microseconds
* Some important log files
    - frame_generator/encode: Time to encode
    - frame_generator/generate_frame_parity_pkts: Time to generate frame parity packets
    - frame_generator/generate_frame_pkts: Time to generate frame data packets
    - receiver/can_decode: Time taken in can_decode
    - receiver/decode: Time taken in decode
    - receiver/qr: QR score
    - receiver/qualityReporter_->generate_quality_report: Time taken to generate QR score
    - receiver/receieve_pkt: End to end time taken by receive_pkt function
    - receiver/received_frames_log: Information about received frames
    - receiver/received_frames_loss_info: indices, packet_losses, frame_losses
    - receiver/RS_FEC: Time taken in Reed Solomon FEC (decode portion). This is a subset of decode time
    - sender/data_pkt: Sizes of data packets and their frame numbers
    - sender/handle_feedback: Time taken in handle feedback
    - sender/next_frame: time taken by next frame
    - sender/par_pkt: Sizes of parity packets and their frame numbers
    - sender/send_pkt: Timestamp when packet was sent and frame number



### Python installations
```
pip install notebook
```


### Config File Parameters
* tau: Default 3
* ge_gMin_pkt: Default 1. If the Strategy is density Match, then this is used for GE Parameter Deduction
* max_data_pkts_per_group: Default 64 Maximum Data packets per group 
* max_fec_pkts_per_group: Default 32 Maximum FEC packets per group 
* strategy_name: Which Strategy to use for deducing GE parameters
* coding_scheme: Which Coding Scheme to use. ALL value in this field will run for all the Coding Schemes
* log_folder: Root folder for logs
* model: Name of the model
* model_means: Model means file
* model_vars: Model vars file
* trace_folder: Folder containing trace file
* port: Default 8080. Port used by receiver
* one_way_delay: Default 50 Delay added by Mahimahi in ms
