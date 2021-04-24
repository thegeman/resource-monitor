
#include "daemon.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


static const char *pid_filename;

/**
 * Creation/destruction of PID file
 */
static void create_pid_file(monitor_options_t *opts) {
	// Use the POSIX open function to atomically check that the PID
	// file does not yet exist and create it
	int pid_fd;
	if ((pid_fd = open(opts->pid_file, O_WRONLY | O_CREAT | O_EXCL,
			S_IRUSR | S_IWUSR)) < 0) {
		printf("Failed to create PID file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	// Write this process's identifier to the PID file
	FILE *pid_file = fdopen(pid_fd, "w");
	fprintf(pid_file, "%d", getpid());
	fclose(pid_file);

	// Save PID filename for later reference to clean up
	pid_filename = opts->pid_file;
}

static void destroy_pid_file() {
	unlink(pid_filename);
}

/**
 * Fork a daemon process to allow the resource monitor to keep running
 */
void daemonize(monitor_options_t *opts) {
	// Create a log file before forking
	// (daemon cannot report errors in log file creation)
	int log_file_fd = open(opts->log_file, O_CREAT | O_WRONLY | O_TRUNC, 0664);
	if (log_file_fd < 0) {
		fprintf(stderr, "Failed to open log file: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Fork a new process
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Failed to fork a daemon process: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		// Succesfully forked, so exit the main process
		printf("Started the daemon process with PID %d.\n", pid);
		printf("To stop the daemon, send a SIGTERM signal using:\n");
		printf("    kill %d\n", pid);
		printf("To force the daemon to flush to disk, send a SIGUSR1 signal using:\n");
		printf("    kill -SIGUSR1 %d\n", pid);
		exit(EXIT_SUCCESS);
	}

	// Close the stdin/out/err handles and assign stdout to the log file
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	if(dup2(log_file_fd, STDOUT_FILENO) < 0) {
		exit(EXIT_FAILURE);
	}
	close(log_file_fd);

	// Get a new session ID to detach from the parent
	pid_t sid = setsid();
	if (sid < 0) {
		printf("Failed to detach from parent process.\n");
		printf(strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Change the file mask for all monitoring output files
	umask(0);

	// Create a PID file and make sure the file is cleaned up on exit
	create_pid_file(opts);
	atexit(destroy_pid_file);
}
