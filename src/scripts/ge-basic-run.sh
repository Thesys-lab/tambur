#!/bin/bash

usage() {
    echo "ge_basic_strategy.sh --config=<config.json>"
    exit 1
}

for i in "$@"; do
  case $i in
    --config=*)
      CONFIG="${i#*=}"
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

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export ROOT_DIR=$SCRIPT_DIR/../..

if [[ -z "${CONFIG}" ]]; then
  usage
else
  export trace_file=$ROOT_DIR/traces/$(jq -r '.trace' "${CONFIG}" )
  export host=$(jq -r '.host' "${CONFIG}" )
  export port=$(jq -r '.port' "${CONFIG}" )
  export one_way_delay=$(jq -r '.one_way_delay' "${CONFIG}" )
  export tau=$(jq -r '.tau' "${CONFIG}" )
  export strategy=$(jq -r '.strategy' "${CONFIG}" )

  # Go through trace
  export ge_params=$($ROOT_DIR/src/fec_examples/get_ge_params $CONFIG $trace_file)

  echo $ge_params

  # Kill any existing receiver
  prev_recv_pid=$(ps -ef | awk '/[f]ec_examples\/multi_receiver/{print $2}')
  if [[ -n "${prev_recv_pid}" ]]; then
    kill -9 $prev_recv_pid
  fi

  # Start new receiver
  $ROOT_DIR/src/fec_examples/multi_receiver $port &
  receiver_pid=$!

  # Setup mahimahi -> Start sender
  mm-delay $one_way_delay mm-ge-loss $ge_params sh -c 'echo $ROOT_DIR $MAHIMAHI_BASE $port $trace_file; $ROOT_DIR/src/fec_examples/sender_trace $MAHIMAHI_BASE $port $trace_file'

fi;