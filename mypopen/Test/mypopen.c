/**
* @file mypopen.c
* Betriebssysteme Beispiel 2
* mypopen - vereinfachte Version der Linux Funktionen "popen und pclose"
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
/*
* --------------------------------------------------------------- defines --
*/
#define SUCCESS 0
#define ERROR 1
/*
* -------------------------------------------------------------- typedefs --
*/
enum Bool { FALSE, TRUE };
/*
* ------------------------------------------------------------- functions --
*/

/**
* \brief initiate a pipe stream to or from a process
*
* \param command the command to be executed
* \param type the mode for I / O (r or w)
*
* \return a file pointer or NULL in case of error
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

int mypclose(FILE* stream) {
	pid_t wait_pid;
}