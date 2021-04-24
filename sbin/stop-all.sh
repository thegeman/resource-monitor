#!/usr/bin/bash

# Get path of RESOURCE_MONITOR_HOME
RESOURCE_MONITOR_HOME="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

# Load the configuration
. $RESOURCE_MONITOR_HOME/etc/load-env.sh

# Stop the daemon on each machine in the list
for machine in $MACHINES; do
	ssh $machine "kill \$(cat \"$PID_FILE\")"
done
