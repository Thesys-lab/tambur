#!/bin/bash

usage() {
    echo "generate_all_plots.sh --config= --plot-folder="
    exit 1
}

for i in "$@"; do
  case $i in
    --config=*)
      CONFIG="${i#*=}"
      shift # past argument=value
      ;;
      --plot-folder=*)
      PLOT="${i#*=}"
      shift # past argument=value
      ;;
    #--default)
    #  DEFAULT=YES
    #  shift # past argument with no value
    #  ;;
    *)
      usage
      ;;
  esac
done

if [[ -z "${CONFIG}" || -z "${PLOT}" ]]; then
  usage
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export LOG=$(jq -r '.log_folder' "${CONFIG}" )

mkdir $PLOT
# can skip coding schemes for some scripts
# some scripts have other arguments that
# one can directly change in bash
python3 $SCRIPT_DIR/cdf_non_recoverable_frames.py --log $LOG --config $CONFIG --output $PLOT/cdf_recoverable_frames.svg --skip StreamingCode_3-point_9
python3 $SCRIPT_DIR/decode_latency_end_to_end.py --log $LOG --config $CONFIG --output $PLOT/cdf_decode_latency_end_to_end.svg --skip StreamingCode_3-point_9
python3 $SCRIPT_DIR/cdf_bandwidth_overhead.py --log $LOG --config $CONFIG --output $PLOT/cdf_bandwidth_overhead.svg --skip StreamingCode_3-point_9
python3 $SCRIPT_DIR/encode-decode.py --log $LOG --config $CONFIG --encode_output $PLOT/encode_latency.svg --decode_output $PLOT/decode_latency.svg --skip ReedSolomonMultiFrame_3 --skip StreamingCode_3-high-BW --skip StreamingCode_3-point_9
python3 $SCRIPT_DIR/decode_latency_frames.py --log $LOG --config $CONFIG --output $PLOT/bar_decode_latency_frames.svg --max_frames 3 --skip StreamingCode_3-point_9
python3 $SCRIPT_DIR/burst_losses.py --log $LOG --config $CONFIG --output $PLOT/mean_recovered_frames_bursts.svg --mean --max_burst 4 --offset -4 --skip StreamingCode_3 --skip StreamingCode_3-point_9
python3 $SCRIPT_DIR/freeze.py --log $LOG --config $CONFIG --output $PLOT/freeze.svg --skip StreamingCode_3-point_9