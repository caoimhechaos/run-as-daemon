#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/wait.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_VFORK_H
#include <vfork.h>
#endif

#include <getopt.h>

#define LOGGER_PATH "/usr/bin/logger"

int optind;

int main(int argc, char **argv)
{
	char *logfile = "/dev/null", *pidfile = NULL;
	char *bname, *strpid;
	int optc, fd, nullfd, to_logger = 0;
	pid_t forkres, mypid;

	while ((optc = getopt(argc, argv, "l:p:")) != -1)
	{
		switch (optc)
		{
		case 'l':
			if (strcmp(optarg, "=logger") == 0)
			{
				logfile = NULL;
				to_logger = 1;
			}
			else
			{
				logfile = optarg;
				to_logger = 0;
			}
			break;
		case 'p':
			pidfile = optarg;
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", optc);
			exit(EXIT_FAILURE);
		}
	}

	if (optind == argc)
	{
		fputs("Please specify a program to run!\n", stderr);
		exit(EXIT_FAILURE);
	}

	/* Find the acutal short program name. */
	bname = rindex(argv[optind], '/');

	if (bname == NULL)
		bname = argv[optind];
	else
		bname++;

	if (!pidfile)
	{
		if (asprintf(&pidfile, "/var/run/%s.pid", bname) < 0)
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}
	}

	nullfd = open("/dev/null", O_RDWR, 0644);
	if (nullfd == -1)
	{
		perror("/dev/null");
		exit(EXIT_FAILURE);
	}

	forkres = vfork();
	if (forkres == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (forkres != 0)
	{
		int exitcode;
		/*
		 * We're the parent; just wait if our kid exits immediately,
		 * then let it have a life of its own.
		 */
		sleep(1);
		if (waitpid(forkres, &exitcode, WNOHANG))
		{
			/* The poor little child died! */
			if (WIFSIGNALED(exitcode))
			{
				fprintf(stderr,
					"%s terminated immediately by %s%s\n",
					bname, strsignal(WSTOPSIG(exitcode)),
					WCOREDUMP(exitcode) ?
					" (core dumped)" : "");
			}
			else if (WIFSTOPPED(exitcode))
			{
				fprintf(stderr, "%s stopped immediately\n",
					bname);
			}
			else if (WIFEXITED(exitcode))
			{
				fprintf(stderr, "%s exited with code %d\n",
					bname, WEXITSTATUS(exitcode));
			}
			else
			{
				fprintf(stderr,
					"Something unknown happened to %s\n",
					bname);
			}
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}

	/* Put the PID into a file. Create a new session at the same time. */
	mypid = setsid();
	if (asprintf(&strpid, "%d", mypid) < 0)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fd = open(pidfile, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	if (fd == -1)
	{
		perror(pidfile);
		exit(EXIT_FAILURE);
	}
	write(fd, strpid, strlen(strpid));
	close(fd);

	if (to_logger)
	{
		int pipefd[2];
		pid_t kid;

		if (pipe(pipefd))
		{
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		kid = vfork();
		if (kid == -1)
		{
			perror("fork");
			exit(EXIT_FAILURE);
		}
		else if (kid == 0)
		{
			/* We're the kid! */
			close(pipefd[1]);
			close(STDOUT_FILENO);
			dup2(nullfd, STDOUT_FILENO);
			close(STDERR_FILENO);
			dup2(nullfd, STDERR_FILENO);

			/* Redirect STDIN to the parent process. */
			close(STDIN_FILENO);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			close(nullfd);

			execl(LOGGER_PATH, LOGGER_PATH, "-i", "-t", bname,
				NULL);
			exit(EXIT_FAILURE);
		}
		else
		{
			/* We're the parent. */
			close(pipefd[0]);
			fd = pipefd[1];
		}
	}
	else
	{
		fd = open(logfile, O_CREAT|O_WRONLY|O_APPEND, 0600);
	}

	close(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);

	close(STDERR_FILENO);
	dup2(fd, STDERR_FILENO);

	close(STDIN_FILENO);
	dup2(nullfd, STDIN_FILENO);

	close(fd);
	close(nullfd);

	execv(argv[optind], &argv[optind]);
	perror("execv");

	exit(EXIT_FAILURE);
}
