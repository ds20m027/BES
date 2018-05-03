/**
* @file mypopen.c
* Betriebssysteme Beispiel 2
* mypopen - ein Clone der Linux Funktionen "popen und pclose"
*
* @author Ralf Ziefuhs <ic17b065@technikum-wien.at>
* @author Clemens Fritzsche <ic17b087@technikum-wien.at>
*
* @date 2018/05/04
*
* @version 1.0
*
*/

/*
* -------------------------------------------------------------- includes --
*/
#include "mypopen.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
/*
* --------------------------------------------------------------- defines --
*/
/*
* -------------------------------------------------------------- typedefs --
*/
/*
* ------------------------------------------------------------- functions --
*/

/**
* \brief popen clone
*
* This function creates a child process executing the command given in
* \a cmd, opens a unnamed pipe, redirects stdin (if \a type is equal
* to "w") or stdout (if \a type is equal to "r") of the child
* process to the pipe and returns a file pointer to the pipe end, that
* is not used by the child process to the caller.
*
* \param command command to execute in the context of the child process
* \param type "r" or "w" depending on whether the parent process
*        intends to read from or write to the pipe.
*
* \return file pointer to the parent's pipe end or NULL in case of an error
*/

FILE* mypopen(const char* command, const char* type) {
	
	int pipefd[2];
	int copy_pipefd[2];
	pid_t cpid;
	char buf;

	if (pipe(pipefd) == -1) {
		return NULL; /*errno wird schon innerhalb von pipe zugewiesen*/
	}

	if (cpid >= 0) { /*überprüft ob nicht schon ein Child Prozess läuft*/
		errno = EAGAIN;
		return NULL;
	}

	cpid = fork();
	
	if (cpid == -1) {
		return NULL; /*errno wird schon innerhalb von pipe zugewiesen*/
	}

	if (cpid == 0) {    /* Child*/
		dup2(pipefd[0], copy_pipefd[0]); /* Creates copy pipefd to new_pipefd */
		dup2(pipefd[1], copy_pipefd[1]);

		if (strcmp(type, "r") == 0) {
			close(pipefd[1]);	/* Close unused write end */
		}

		if (strcmp(type, "w") == 0) {
			close(pipefd[0]);	/* Close unused read end */
		}

		else {
			errno = EINVAL; /*invalid value*/
			return NULL;
		}
		

		while (read(pipefd[0], &buf, 1) > 0)
			write(STDOUT_FILENO, &buf, 1);

		write(STDOUT_FILENO, "\n", 1);
		close(pipefd[0]);
		exit(EXIT_SUCCESS);

	}
	/*else {            // Parent /
		close(pipefd[0]);          // Close unused read end /
		write(pipefd[1], argv[1], strlen(argv[1]));
		close(pipefd[1]);          // Reader will see EOF 
		wait(NULL);                // Wait for child /
		exit(EXIT_SUCCESS);/
	}*/
}



/**
* \brief pclose clone
*
* This function closes the pipe, waits for the child process to exit
* and retrieves and returns the exit status of the child.
*
* \param stream file pointer to parent's pipe end
*
* \return exit status of child process or -1 in case of an error
*/

int mypclose(FILE* stream) {
	pid_t wait_pid;
}

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: