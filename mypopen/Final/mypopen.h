/**
* @file mypopen.h
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

#ifndef _MYPOPEN_H_

#define _MYPOPEN_H_

/*
* -------------------------------------------------------------- includes --
*/
#include <stdio.h>
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
extern FILE *mypopen(const char *command, const char *type);

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
extern int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: