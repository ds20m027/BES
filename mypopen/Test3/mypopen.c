/**
 * Betriebssysteme mypopen Library
 * Beispiel 2
 *
 * @author Gerald Karpiel <ic15b081@technikum-wien.at>
 * @author Thomas Wöpperer <ic15b503@technikum-wien.at>
 * @author Wolfgang Weißinger <ic15b073@technikum-wien.at>
 * 
 * @date 21/04/2016
 *
 * @file mypopen.c
 * @version 1.x
 *
 *	similar to popen and pclose - open and close pipes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <wait.h>

/*
* -------------------------------------------------------------- include own h-Files --
*/
#include "mypopen.h"

/*
* -------------------------------------------------------------- typedefs --
*/
/*
* ### FB: Blanc ist wahrscheinlich hineingerutscht. Mit dem Define ERROR wuerde ich aber aufpassen 
*         GRUND: Es kann sein das dies bei manchen Compiler ein Schluesselwort ist. (Besser HELP_DEBUG oder aehnliches ...)  
* ### FB BP: Whitespace nach dem # und vor den Präprozessor-Keywords sind keine Fehler - es gab auch mal Präprozessoren, da mußte
*            das '#' in der 1. Spalte sein (und mit den Whitespaces kann man dann verschachtelte Präprozessoranweisungen
*            übersichtlicher formatieren).
*            Und bei welchem C-Compiler soll das ein Keyword gewesen sein?
*            "Vordefinierte" #define's sollten hier BTW Fehler werfen ....
*/                  
// ### FB BP: Genau derartiges mach' ich auch - nur daß ich mehr als eins davon hab;-)
# define ERROR 0 // Set 1 to include error messages for debug

/*
* --------------------------------------------------------------- globals --
*/
static FILE *fp = NULL;
static pid_t fpid = -1;

/*
* ------------------------------------------------------------- functions --
*/

/**
 * \brief mypopen function similar to popen function
 *
 * \param command - bash command for whitch pipe is opened
 * \param modus - "r" or "w" direction of pipe 
 *
 * \return NULL if a error occured filepointer onseccess
*/
/*
* ### FB: Name der Variablen ist zwischen H-File und C-Source Unterschiedlich --> Kein Fehler aber unschoen
*         Im H-File:  extern FILE *mypopen(
*                                         const char *cmd,
*                                         const char *type
*                                         );
*/                  
FILE *mypopen(const char *command, const char *modus)
{
/* ### FB: Eine Default-Initialisierung waere hier zumindest schoen (nicht unbedingt notwendig) */      
	FILE *fop;
	int pdes[2]; 
/* ### FB: Ein Define fuer spaeteren Zugriff waere vielleicht besser lesbar aber das ist eine Stilfrage */      
/*
 *	pdes 0 refers to read end of the pipe
 *	pdes 1 refers to write end of the pipe
*/
	pid_t pid;
	
	if(fpid != -1)
	{
		errno = EAGAIN;
		return NULL;
	}

	if(command == NULL)
	{
        errno = EINVAL;
#if ERROR
        fprintf(stderr, "Invalid Argument!\n");
#endif
		return NULL;
	}
	
	if(modus == NULL)
	{
        errno = EINVAL;
#if ERROR
        fprintf(stderr, "Invalid Argument!\n");
#endif
		return NULL;
	}

	if((strcmp(modus, "r") != 0) && (strcmp(modus, "w") != 0))
	{
		errno = EINVAL;
#if ERROR
        fprintf(stderr, "Invalid Argument!\n");
#endif
		return NULL;
	}

    if(pipe(pdes)==-1)
    {
#if ERROR
        fprintf(stderr, "%s\n", strerror(errno));
#endif
// ### FB BP: Oh je, wir haben keien Pipe und machen trotzdem weiter? [-2]
// {-1}
    }

	pid = fork();
	if(pid == -1)
	{
#if ERROR
        fprintf(stderr, "%s\n", strerror(errno));
#endif
/*
* ### FB: Da close den die errno noch veraendern kann (siehe MAN-Spec) sollte hier die Errno gesichert werden
*         da sonst der eigentliche Fehler ueberschrieben werden kann
*         Auszug aus der man Spec. fuer close: 
*            EBADF  fd is not a valid open file descriptor.
*            EINTR  The close() call was interrupted by a signal; see signal(7).
*            EIO    An I/O error occurred.
* ### FB BP: Ja, aber das close(2) hier schließt die Enden einer Pipe und das wird hier nie schief gehen.
*/                  
		close(pdes[0]);
		close(pdes[1]);

		return NULL;
	}
	else if(pid == 0)
	{
		if(modus[0] == 'r')
		{
			close(pdes[0]);

// ### FB BP: Sehr gut! Check, daß wir nicht stdout/stdin schließen! [+2]
			if(pdes[1] != STDOUT_FILENO)
			{
				if(dup2(pdes[1], STDOUT_FILENO) == -1)
				{
					close(pdes[1]);
					exit(EXIT_FAILURE);
				}
				close(pdes[1]);
			}
		}
		else 
		{
			close(pdes[1]);

// ### FB BP: Sehr gut! Check, daß wir nicht stdout/stdin schließen! [+0]
			if(pdes[0] != STDIN_FILENO)
			{
				if(dup2(pdes[0], STDIN_FILENO) == -1)
				{
					close(pdes[0]);
					exit(EXIT_FAILURE);
				}
				close(pdes[0]);
			}
		}

// ### FB BP: Das müßte 100% korrekterweise "(char*)NULL" sein, wie auch in `man 2 execl` nachzulesen ist.
//            Ich zieh' keine Punkte ab, weil ich nicht in beiden Verbänden das explizit erzählt hab. [-0]
		execl("/bin/sh", "sh", "-c", command, NULL);
/* ### FB: Warum wurde dieser EXIT-Status gewaehlt?  Ich haette entweder 127 genommen ("command not found" oder EXIT_FAILURE */      
		exit(3);
	}
	else
	{
		if(modus[0] == 'r')
		{
			fop = fdopen(pdes[0], modus);
			close(pdes[1]);
		} else {

			fop = fdopen(pdes[1], modus);
			close(pdes[0]);
		}

		if(fop == NULL)
		{
/* ### FB: FEHLER was passiert wenn der fop auf NULL ist ? wer schliest das Ende der Pipe ???   'r'=pdes[0] oder 'w'=pdes [1] */      
/// {+2}
		pid = -1;
		}

		fp = fop;
		fpid = pid;
#if ERROR
		printf("Process ID:%d\n", pid);
#endif
/* ### FB: Potentielle spaetere Fehler Quelle. Wuerde das "return fop" eine Zeile (nach }) geben. */      
		 return fop;
	}
}


/**
 * \brief mypopen function similar to popen function
 *
 * \param fop - Filepointer of open pipe
 *
 * \return -1 status on error and status of childprocess success
*/
/*
* ### FB: Name der Variablen ist zwischen H-File und C-Source Unterschiedlich --> Kein Fehler aber unschoen
*         Im H-File:  extern int mypclose(
*                         FILE *fp
*                         );
*/                  
int mypclose(FILE *fop)
{
	int status = -1;
	int pid;
	if(fpid == -1)
	{
		errno = ECHILD;
		return -1;
	}

	if(fop == NULL)
	{	
		errno = EINVAL;
		return -1;
	}

	if(fop != fp)
	{
        errno = EINVAL;
		return -1;
	}

/* ### FB: Dafuer gibt es ein Define EOF                               */      
	if(fclose(fop) == -1)
	{
/* ### FB: Warum wird fp und fpid nicht wieder Initialisiert ? (aus unserer Sicht ein Fehler) */      
// {+2}
		return -1;
	}

	do{
		pid = waitpid(fpid, &status, 0);		
		
	}while(pid ==-1 && errno == EINTR);

	if(!(WIFEXITED(status)))
	{
	errno = ECHILD;
	
#if ERROR
	printf("Current status: %d\n", status);
#endif

	return -1;
	
	}
	if(pid == -1)
		return -1;
	
	fpid = -1;
	fp = NULL;
	
#if ERROR
	printf("Current status: %d\n", status);
#endif

	return WEXITSTATUS(status);
}


