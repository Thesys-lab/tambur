#!/bin/bash

usage() {
    echo "run-config.sh --host=<host> --config=<config.json> [--no-list] [--hc-ge] [--no-mahi]"
    exit 1
}

function get_seed() {
  export seed=$(seq 0 1000000 | shuf -n 1)
}

NOLIST=""
HARD_CODE_GE=""
MAHI_DISABLED=""
export CONFIG=""

for i in "$@"; do
  case $i in
    --config=*)
      CONFIG="${i#*=}"
      shift # past argument=value
      ;;
    --no-list)
     NOLIST="1"
     shift # past argument with no value
     ;;
    --hc-ge)
      HARD_CODE_GE="--hc-ge"
      shift
      ;;
    --no-mahi)
      MAHI_DISABLED="--no-mahi"
      shift
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

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export ROOT_DIR=$SCRIPT_DIR/../..

if [[ -z "${CONFIG}" ]]; then
  usage
fi

export seed=$(jq -r '.seed' "${CONFIG}" )
if [[ "null" == "${seed}" ]]; then
  get_seed
fi

export trace_folder=$ROOT_DIR/$(jq -r '.trace_folder' "${CONFIG}" )
export port=$(jq -r '.port' "${CONFIG}" )
export one_way_delay=$(jq -r '.one_way_delay' "${CONFIG}" )
export log_folder=$(jq -r '.log_folder' "${CONFIG}" )
export coding_scheme=$(jq -r '.coding_scheme' "${CONFIG}" )
export cfg_base_file=$(basename $CONFIG )

if [[ -n "${NOLIST}" ]]; then
  # store the config inside log folder
  mkdir -p $log_folder/$cfg_base_file
  cp $CONFIG $log_folder/$cfg_base_file/
  rm -f $log_folder/$cfg_base_file/GE_params

  for tf in $trace_folder/*; do
      export trace_file=$tf
      # Go through trace
      echo "Running for coding_scheme $coding_scheme trace $trace_file using $CONFIG"
      if [[ -n "${HARD_CODE_GE}" ]]; then
        t_gb=$(jq -r '.t_gb' "${CONFIG}" )
        t_bg=$(jq -r '.t_bg' "${CONFIG}" )
        l_g=$(jq -r '.l_g' "${CONFIG}" )
        l_b=$(jq -r '.l_b' "${CONFIG}" )
        export ge_params="${t_gb} ${t_bg} ${l_g} ${l_b}"
      else
        export ge_params=$($ROOT_DIR/src/fec_examples/get_ge_params $CONFIG $trace_file)
      fi

      echo "Got GE params: seed t_gb_ t_bg_ l_g_ l_b_ $seed $ge_params"

      echo "coding_scheme: $coding_scheme trace: $trace_file cfg: $CONFIG seed: $seed (t_gb_ t_bg_ l_g_ l_b_): $ge_params" >> $log_folder/$cfg_base_file/GE_params

      # Two tries to handle the case where first try fails

      for try in 1 2
      do
        start_time=`date +%s`

        echo "try $try"

        # Start new receiver
        $ROOT_DIR/src/fec_examples/multi_receiver $CONFIG $trace_file &
        receiver_pid=$!

        echo "Receiver started with PID: $receiver_pid on port $port"
        sleep 1

        # Setup mahimahi -> Start sender
        if [ -n "${MAHI_DISABLED}" ]; then
          echo "not using mahi mahi"
          sh -c '$ROOT_DIR/src/fec_examples/sender_trace 127.0.0.1 $CONFIG $trace_file $seed $ge_params'
        else
          echo "Using mahi mahi"
          mm-delay $one_way_delay mm-ge-loss $seed $ge_params sh -c 'echo $MAHIMAHI_BASE; $ROOT_DIR/src/fec_examples/sender_trace $MAHIMAHI_BASE $CONFIG $trace_file'
        fi
        success=$?

        end_time=`date +%s`
        runtime=$((end_time-start_time))

        echo "Time taken: $runtime"

        if [[ "$success" -eq "0" ]]; then
          break;
        else
          echo "Try $try failed"
          sleep 10
        fi
      done

      sleep 10
  done
else
  # Generate new configs
  tmp_time=`date +%s`
  mkdir -p tmp_configs.$tmp_time
  python3 $SCRIPT_DIR/generate_new_configs.py --out_folder  tmp_configs.$tmp_time --config $CONFIG --seed $seed
  $SCRIPT_DIR/run-all-configs.sh --config_folder=tmp_configs.$tmp_time $HARD_CODE_GE $MAHI_DISABLED
  #rm -r tmp_configs
fi
