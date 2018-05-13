/**
* @file mypopen.c
* Betriebssysteme Beispiel 2
* mypopen - ein Klon der Linux Funktionen "popen und pclose"
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
/*
* --------------------------------------------------------------- globals --
*/
static pid_t pid = -1;
static FILE* gl_Stream = NULL;

/* 
* ### FB Grp0: return wert -> wir haben die Angabe so verstanden, dass ein file pointer
* oder Fehlerwerte retourniert werden müssen 
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
FILE* mypopen(const char* command, const char* type)
{
	int fd_pipe[2];

	int child_end;
	int parent_end;
	
	/* ### FB Grp0: If Statements mit den selben return Values in einem If zusammenfassen*/

	//Kontrolle der Parameter
	if (command == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	if (type == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	if (strcmp(type, "r") == 0)
	{
		parent_end = STDIN_FILENO;			//parent_end = Leseende fd_Pipe[0]
		child_end = STDOUT_FILENO;			//child_end = Schreibende fd_Pipe[1]
	}
	else if (strcmp(type, "w") == 0)
	{
		parent_end = STDOUT_FILENO;			//parent_end = Schreibende fd_Pipe[1]
		child_end = STDIN_FILENO;			//child_end = Leseende fd_Pipe[0]
	}
	else
	{
		errno = EINVAL;
		return NULL;
	}
	
	/* ### FB Grp0: If Statements mit den selben return Values in einem If zusammenfassen*/

	if (pid != -1)
	{
		errno = EAGAIN;
		return NULL;
	}

	if (gl_Stream != NULL)
	{
		errno = EAGAIN;
		return NULL;
	}

	//Pipe wird erzeugt
	if (pipe(fd_pipe) == -1)
	{
		return NULL;						//errno wird durch pipe() gesetzt
	}

	//Neuer Prozess wird erzeugt	
	switch (pid = fork())
	{
	
	//-------------Error---------------
	case -1:
		if (close(fd_pipe[parent_end]) == -1)
		{
			return NULL;
		}
		if (close(fd_pipe[child_end]) == -1)
		{
			return NULL;
		}
		errno = EAGAIN;
		return NULL;
		/* ### FB Grp0: Break Command ist nicht notwendig da dieses niemals erreicht werden kann (unreachable code)*/
		break;
		
	//--------------Child---------------
	case 0:
		if (close(fd_pipe[parent_end]) == -1)
		{
			return NULL;					//errno wird durch close() gesetzt
		}

		if (fd_pipe[child_end] != child_end)
		{

			if (dup2(fd_pipe[child_end], child_end) == -1)
			{
				if (close(fd_pipe[child_end]) == -1)
				{
					return NULL;			//errno wird durch close() gesetzt
				}
				_exit(EXIT_FAILURE);
			}

			if (close(fd_pipe[child_end]) == -1)
			{
				return NULL;				//errno wird durch close() gesetzt
			}
		}

		execl("/bin/sh", "sh", "-c", command, (char*)NULL);

		_exit(EXIT_FAILURE);				//wird nur erreicht wenn execl nicht funktioniert
		/* ### FB Grp0: Break Command ist nicht notwendig da dieses niemals erreicht werden kann (unreachable code)*/
		break;

	//--------------Parent---------------
	default:
		if (close(fd_pipe[child_end]) == -1)
		{
			return NULL;					//errno wird durch close() gesetzt
		}

		gl_Stream = fdopen(fd_pipe[parent_end], type);
		break;
	}

	if (gl_Stream == NULL)
	{
		pid = -1;

		if (close(fd_pipe[parent_end]) == -1)
		{
			return NULL;					//errno wird durch close() gesetzt
		}
	}

	return gl_Stream;
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
int mypclose(FILE* stream)
{
	if (pid == -1)
	{
		errno = ECHILD;
		return -1;
	}

	/* ### FB Grp0: If Statements mit den selben return Values in einem If zusammenfassen*/
	
	if (gl_Stream == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (gl_Stream != stream)
	{
		errno = EINVAL;
		return -1;
	}

	if (fclose(stream) == EOF)
	{
		gl_Stream = NULL;
		pid = -1;
		return -1;							//errno wird durch fclose() gesetzt
	}

	/* ### FB Grp0: Variablen sollten am Anfang einer Funktion definiert werden.*/
	pid_t wait_pid;
	int status;

	//wartet bis der Child Prozess terminiert
	while ((wait_pid = waitpid(pid, &status, 0)) != pid)
	{
		if (wait_pid == -1)
		{
			if (errno == EINTR)
			{
				continue;					//falls waitpid() unterbrochen
			}

			return -1;						//errno wird durch wait_pid() gesetzt
		}
	}

	//Globale Variablen werden zurückgesetzt
	gl_Stream = NULL;
	pid = -1;

	if (WIFEXITED(status) != 0)
	{
		return WEXITSTATUS(status);
	}

	errno = ECHILD;
	return -1;
}

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: