# Get path of RESOURCE_MONITOR_HOME
RESOURCE_MONITOR_HOME="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/..")"

# Load the configuration
. $RESOURCE_MONITOR_HOME/etc/resource-monitor.conf

# Check mandatory configuration options and set optional options
if [ -z "$MACHINES" ]; then
	echo "Missing mandatory configuration option: MACHINES" >&2
	exit -1
fi
METRIC_DIR="${METRIC_DIR:-"."}"
LOG_FILE="${LOG_FILE:-"resource-monitor-\$(hostname).log"}"
PID_FILE="${PID_FILE:-"/tmp/resource-monitor.pid"}"
