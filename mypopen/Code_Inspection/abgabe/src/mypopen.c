/**
* @file mypopen.c
* "Betriebssysteme mypopen"
* Beispiel 2
*
* @author Thomas Zeitlhofer <ic17b031@technikum-wien.at>
* @author Niklas Hohenwarter <ic17b008@technikum-wien.at>
* @author Sebastian Scheiber <ic17b084@technikum-wien.at>
* @date 2018/04/30
* @version 1
*/

// -------------------------------------------------------------- includes --

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------- defines --

#define READ_PIPE 0
#define WRITE_PIPE 1

// --------------------------------------------------------------- globals --

static pid_t global_pid = -1;
static FILE *global_file_pointer = NULL;

// ------------------------------------------------------------- functions --

/* ### FB_Grp13: return wert -> file pointer oder NULL im Fehlerfall */
/**
* \brief
* mypopen uses fork to create a child process and sets up a pipe
*
* @param command (const char*)
* @param type (const char*)
* @return file pointer on pipe
* @version 1
* @date 2018/04/30
*/

FILE *mypopen(const char *command, const char *type)
{
    int file_descriptor[2];

	/* ### FB_Grp13: keine Abrage von *global_file_pointer auf NULL */
    if(global_pid >= 0) // check if a child process is open
    {
        errno = EAGAIN; // Resource temporarily unavailable
        return NULL;
    }

	/* ### FB_Grp13: keine Abfrage des Parameter command auf NULL */
    if((type == NULL) || (type[1] != '\0') || (type[0] != 'r' && type[0] != 'w'))
    {
        errno = EINVAL; // Invalid argument (POSIX.1-2001).
        return NULL;
    }

    if(pipe(file_descriptor) == -1) // check if pipe was able to open its two ends
    {
        return NULL;
    }

    switch(global_pid = fork())
    {
    case -1: // fork failed
		/* 
		 * ### FB_Grp13: keine Fehlerbehandlung beim Schlieﬂen des file descriptors
		 *               Funktion close(2) liefert im Fehlerfall -1
		 */
		(void) close(file_descriptor[WRITE_PIPE]); //clean up write end
        (void) close(file_descriptor[READ_PIPE]); //clean up read end
		/*
		 * ### FB_Grp13: errno muss manuell gesetzt werden da es von close(2) ¸berschrieben wird
		 *               errno = EAGAIN;		 
		 */
        return NULL;
		/* ### FB_Grp13: bei einem switch case ist ein break; zu setzen */

    case 0: //child
        if(type[0] == 'r')
        {
			/*
			* ### FB_Grp13: keine Fehlerbehandlung beim Schlieﬂen des file descriptors
			*               Funktion close(2) liefert im Fehlerfall -1
			*/
			(void)close(file_descriptor[READ_PIPE]); // parent read end not needed in child process
            if(file_descriptor[WRITE_PIPE] != STDOUT_FILENO) // check if STDOUT_FILENO is already the child pipe write end
            {
                if(dup2(file_descriptor[WRITE_PIPE], STDOUT_FILENO) == -1) // change STDOUT_FILENO to child pipe write end
                {
                    (void) close(file_descriptor[WRITE_PIPE]); // if dup2 failed close child write pipe end for clean up
                    _Exit(EXIT_FAILURE); // terminate process imediately
                }
                (void) close(file_descriptor[WRITE_PIPE]); // child pipe write end close after successful dup2()
            }
            (void) execl("/bin/sh", "sh", "-c", command, (char *) NULL); // execute command
            _Exit(EXIT_FAILURE); // terminates if execl is not successful - should not be reached
        }
        else if(type[0] == 'w')
        {
            close(file_descriptor[WRITE_PIPE]); // parent write end not needed in child process
            if(file_descriptor[READ_PIPE] != STDIN_FILENO) // check if STDIN_FILENO is already the child pipe read end
            {
                if(dup2(file_descriptor[READ_PIPE], STDIN_FILENO) == -1) // change STDIN_FILENO to child read pipe end
                {
                    (void) close(file_descriptor[READ_PIPE]); // if dup2 failed close child read pipe end for clean up
                    _Exit(EXIT_FAILURE); // terminate process imediately
                }
                (void) close(file_descriptor[READ_PIPE]); // child pipe read end close after successful dup2()
            }
            (void) execl("/bin/sh", "sh", "-c", command, (char *) NULL); // execute command
            _Exit(EXIT_FAILURE); // terminates if execl is not successful - should not be reached
        }
		/* ### FB_Grp13: bei einem switch case ist ein break; zu setzen */

    default: // parent
        if(strcmp(type, "r") == 0)
        {
			/*
			* ### FB_Grp13: keine Fehlerbehandlung beim Schlieﬂen des file descriptors
			*               Funktion close(2) liefert im Fehlerfall -1
			*/
			(void)close(file_descriptor[WRITE_PIPE]); // child write pipe end not needed in parent process
            global_file_pointer = fdopen(file_descriptor[READ_PIPE], "r"); // transform file descriptor to file pointer
            if (global_file_pointer == NULL) // check if transformation was successful
            {
                (void) close(file_descriptor[READ_PIPE]); // clean up parent read pipe end
                global_pid = -1; // reset global_pid
				/* ### FB_Grp13: return Null ist hier nicht notwendig, da sowieso return global_file_pointer */
				return NULL;
            }
        }
        else if(strcmp(type, "w") == 0)
        {
            (void)close(file_descriptor[READ_PIPE]); // child read pipe end not needed in parent process
            global_file_pointer = fdopen(file_descriptor[WRITE_PIPE], "w"); // transform file descriptor to file pointer
            if (global_file_pointer == NULL) // check if transformation was successful
            {
                (void) close(file_descriptor[WRITE_PIPE]); // clean up parent write pipe end
                global_pid = -1; // reset global_pid
				/* ### FB_Grp13: return Null ist hier nicht notwendig, da sowieso return global_file_pointer */
				return NULL;
            }
        }
        break;
    }
    return global_file_pointer;
}


/* ### FB_Grp13: return ist exit status oder -1 im Fehlerfall */
/**
* \brief
* mypclose closes file pointer on pipe
*
* @param stream (FILE*)
* @return exit status or error
* @version 1
* @date 2018/04/30
*/

int mypclose(FILE *stream)
{
    int status;
    pid_t wait_pid;

    if(global_pid < 0) // check if no child process is open
    {
        errno = ECHILD; // No child processes (POSIX.1-2001).
        return -1;
    }

    if(global_file_pointer != stream) //check if correct filepointer was provided
    {
        errno = EINVAL; // Invalid argument (POSIX.1-2001).
        return -1;
    }

    if(fclose(stream) != 0)
    {
        global_file_pointer = NULL; // reset global_file_pointer to prevent another fclose() call and undefined behavior
        global_pid = -1; // reset global_pid
        return -1;
    }

    while ((wait_pid = waitpid(global_pid, &status, 0)) != global_pid)
    {
        if(wait_pid == -1)
        {
            if(errno == EINTR) // check if errno is interrupt
            {
                continue; // if interrupt continue wait
            }
            errno = ECHILD; // No child processes (POSIX.1-2001).
            global_file_pointer = NULL; // reset global_file_pointer
            global_pid = -1; // reset global_pid
            return -1;

        }
    }

    global_file_pointer = NULL; // reset global_file_pointer
    global_pid = -1; // reset global_pid

    if(!(WIFEXITED(status))) // check if child did not terminated normally
    {
        errno = ECHILD; // No child processes (POSIX.1-2001).
        return -1; 

    }
    return WEXITSTATUS(status); // child terminated normally return
}

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End:
