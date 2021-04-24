#!/usr/bin/bash

# Check for presence of argument(s)
if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <output-directory>" >&2
	exit -1
fi
OUTPUT_DIRECTORY="$1"

# Get path of RESOURCE_MONITOR_HOME
RESOURCE_MONITOR_HOME="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

# Load the configuration
. $RESOURCE_MONITOR_HOME/etc/load-env.sh

# Copy the metrics directory on every machine
echo "Flushing metric files (if monitor is active)..."
for machine in $MACHINES; do
	ssh $machine "if [ -e \"$PID_FILE\" ]; then kill -SIGUSR1 \$(cat \"$PID_FILE\"); fi"
done
sleep 1

# Copy the metrics directory on every machine
echo "Copying metric data..."
for machine in $MACHINES; do
	scp "$machine:$METRIC_DIR/*" "$OUTPUT_DIRECTORY/"
done
