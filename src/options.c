#include "monitor.h"

#include <argp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_OUTPUT_DIRECTORY "."
#define DEFAULT_MONITOR_INTERVAL 100
#define DEFAULT_PID_FILE "/tmp/resource-monitor.pid"
#define STR(X) #X
#define STR2(X) STR(X)

// Description of the program to print on --help
static char doc[] = "A resource monitoring tool for collecting fine-grained resource traces.";

// Define program options
enum LONG_OPTIONS {
	OPTION_NO_CPU = 0x100,
#ifdef CUDA
	OPTION_NO_GPU,
#endif
	OPTION_NO_MEMORY,
	OPTION_NO_NETWORK,
	OPTION_NO_DISK
};

static struct argp_option options[] = {
	{ "output-dir",       'o',               "DIR",  0, "Output directory to store resource traces in [default: " DEFAULT_OUTPUT_DIRECTORY "]" },
	{ "monitor-interval", 'i',               "MS",   0, "Interval between consecutive measurements, in milliseconds [default: " STR2(DEFAULT_MONITOR_INTERVAL) "]" },
	{ "daemon",           'D',               0,      0, "Run monitor as a daemon process [default: false]" },
	{ "pid-file",         'p',               "FILE", 0, "File to write monitoring daemon's PID to [default: " DEFAULT_PID_FILE "]" },
	{ "log-file",         'l',               "FILE", 0, "File to write daemon logs to [default: resource-monitor-$(hostname).log]" },
	{ "no-cpu",           OPTION_NO_CPU,     0,      0, "Disable monitoring of CPU resources" },
#ifdef CUDA
	{ "no-gpu",           OPTION_NO_GPU,     0,      0, "Disable monitoring of GPU resources" },
#endif
	{ "no-memory",        OPTION_NO_MEMORY,  0,      0, "Disable monitoring of memory resources" },
	{ "no-network",       OPTION_NO_NETWORK, 0,      0, "Disable monitoring of network resources" },
	{ "no-disk",          OPTION_NO_DISK,    0,      0, "Disable monitoring of disk resources" },
	{ 0 }
};


// Create parser for program options
static error_t option_parser(int key, char *arg, struct argp_state *state) {
	monitor_options_t *opts = (monitor_options_t *)state->input;
	int arg_as_int;
	// Parse a single option and store it in the output
	switch (key) {
		case 'o': // --output-dir
			opts->output_directory = arg;
			break;
		case 'i': // --monitor-interval
			arg_as_int = atoi(arg);
			if (arg_as_int <= 0) {
				fprintf(stderr, "Monitoring interval must be a postive integer\n");
				return EINVAL;
			}
			opts->monitor_period = arg_as_int * MILLISECONDS;
			break;
		case 'D': // --daemon
			opts->daemon = true;
			break;
		case 'l': // --log-file
			opts->log_file = arg;
			break;
		case 'p': // --pid-file
			opts->pid_file = arg;
			break;
		case OPTION_NO_CPU: // --no-cpu
			opts->enable_cpu_monitoring = false;
			break;
#ifdef CUDA
		case OPTION_NO_GPU: // --no-gpu
			opts->enable_gpu_monitoring = false;
			break;
#endif
		case OPTION_NO_MEMORY: // --no-memory
			opts->enable_memory_monitoring = false;
			break;
		case OPTION_NO_NETWORK: // --no-network
			opts->enable_network_monitoring = false;
			break;
		case OPTION_NO_DISK: // --no-disk
			opts->enable_disk_monitoring = false;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

// Put the argument parser together
static struct argp argp = { options, option_parser, NULL, doc };

// Helper function for calling the argument parser
monitor_options_t parse_command_line(int argc, char **argv) {
	// Format default log file name
	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	hostname[255] = '\0';
	char *default_log_file = (char *)calloc(512, sizeof(char));
	strcpy(default_log_file, "resource-monitor-");
	strcat(default_log_file, hostname);
	strcat(default_log_file, ".log");

	// Default options
	monitor_options_t opts = {
		.output_directory = DEFAULT_OUTPUT_DIRECTORY,
		.monitor_period = DEFAULT_MONITOR_INTERVAL * MILLISECONDS,
		.log_file = default_log_file,
		.pid_file = DEFAULT_PID_FILE,
		.daemon = false,
		.enable_cpu_monitoring = true,
#ifdef CUDA
		.enable_gpu_monitoring = true,
#endif
		.enable_memory_monitoring = true,
		.enable_network_monitoring = true,
		.enable_disk_monitoring = true
	};
	// Parse any command line options
	argp_parse(&argp, argc, argv, 0, 0, &opts);
	// Print the collected options if in debug mode
	DEBUG_PRINT("Monitoring options after parsing the command line:\n");
	DEBUG_PRINT("  output_directory = %s\n", opts.output_directory);
	DEBUG_PRINT("  monitor_period = %llu ns\n", opts.monitor_period);
	DEBUG_PRINT("  daemon = %d\n", opts.daemon);
	DEBUG_PRINT("  log_file = %s\n", opts.log_file);
	DEBUG_PRINT("  pid_file = %s\n", opts.pid_file);
	DEBUG_PRINT("  enable_cpu_monitoring = %d\n", opts.enable_cpu_monitoring);
#ifdef CUDA
	DEBUG_PRINT("  enable_gpu_monitoring = %d\n", opts.enable_gpu_monitoring);
#endif
	DEBUG_PRINT("  enable_memory_monitoring = %d\n", opts.enable_memory_monitoring);
	DEBUG_PRINT("  enable_network_monitoring = %d\n", opts.enable_network_monitoring);
	DEBUG_PRINT("  enable_disk_monitoring = %d\n", opts.enable_disk_monitoring);

	// Clean up default_log_file buffer if needed
	if (opts.log_file != default_log_file) {
		free(default_log_file);
	}

	return opts;
}
