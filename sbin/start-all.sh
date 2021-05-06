#!/usr/bin/bash

# Get path of RESOURCE_MONITOR_HOME
RESOURCE_MONITOR_HOME="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

# Load the configuration
. $RESOURCE_MONITOR_HOME/etc/load-env.sh

# Start the daemon on each machine in the list
for machine in $MACHINES; do
	ssh $machine "$RESOURCE_MONITOR_HOME/bin/resource-monitor" \
		-D \
		-p "$PID_FILE" \
		-o "$METRIC_DIR" \
		-l "$LOG_FILE" >/dev/null
done
