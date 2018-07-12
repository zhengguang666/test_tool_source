#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/signal.h>

#define MAX_BUF_SIZE	(10*1024)

char buf[MAX_BUF_SIZE];

// Global variables that control process shutdown.
sig_atomic_t graceful_quit = 0;

// Signal handler for SIGINT.
void SIGINT_handler (int signum)
{
	assert (signum == SIGINT);
	graceful_quit = 1;
}

// Signal handler for SIGQUIT.
void SIGQUIT_handler (int signum)
{
	assert (signum == SIGQUIT);
	graceful_quit = 1;
}

main(int argc)
{
	struct timeval tv;
	double time_start,time_now,time_prev;
	int first = 1;
	struct sigaction sa;

	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;

	// as of now no arguments are allowed. print usage
	if (argc != 1)
	{
		printf(
			"Timestamps each line coming over stdin\n"
			"\tUSAGES:\n"
			"\t\ttstamp.exe < /dev/ttyS0\n"
			"\t\t<commands> | tstamp.exe\n"
			"\t\tetc..\n"
			"\n"
			"Output is printed in the following format\n"
			"\tcolumn1 is elapsed time since first message\n"
			"\tcolumn2 is elapsed time since previous message\n"
			"\tcolumn3 is the message\n"
		);

		exit(0);
	}

	// Register the handler for SIGINT.
	sa.sa_handler = SIGINT_handler;
	sigaction (SIGINT, &sa, 0);

	// Register the handler for SIGQUIT.
	sa.sa_handler =  SIGQUIT_handler;
	sigaction (SIGQUIT, &sa, 0);

	printf(
		"\tcolumn1 is elapsed time since first message\n"
		"\tcolumn2 is elapsed time since previous message\n"
		"\tcolumn3 is the message\n"
	);

	while (graceful_quit == 0)
	{
		if (fgets(buf, MAX_BUF_SIZE, stdin))
		{
			// get system time
			gettimeofday(&tv, NULL);

			// convert to double
			time_now = tv.tv_sec + (tv.tv_usec*(1.0/1000000.0));

			// if first time, notedown the time_start
			if (first)
			{
				first = 0,  time_start = time_prev = time_now;
			}

			fprintf(stdout
				,"%03.3f %02.3f: %s"
				,(float)(time_now-time_start)
				,(float)(time_now-time_prev)
				,buf
			);

			time_prev = time_now;
		}
	}

	fflush(stdout);
	exit(0);
}
