#!/bin/bash

usage() {
    echo "run-all-configs.sh --config_folder=<path>"
    exit 1
}

HARD_CODE_GE=""
MAHI_DISABLED=""

for i in "$@"; do
  case $i in
    --config_folder=*)
      CONFIG_FOLDER="${i#*=}"
      shift
      ;;
    --hc-ge)
      HARD_CODE_GE="--hc-ge"
      shift
      ;;
     --no-mahi)
      MAHI_DISABLED="--no-mahi"
      shift
      ;;
    *)
      usage
      ;;
  esac
done

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
export ROOT_DIR=$SCRIPT_DIR/../..

if [[ -z "${CONFIG_FOLDER}" ]]; then
  usage
fi

for cfg in $CONFIG_FOLDER/*; do
    # Go through trace
    echo "Running for config $cfg"
    $SCRIPT_DIR/run-config.sh --config=$cfg --no-list $HARD_CODE_GE $MAHI_DISABLED
    sleep 10
done