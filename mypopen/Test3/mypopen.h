/* ### FB BP: Doxygen-File-Header fehlt.
 *            -> https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html#doxygen-file
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <wait.h>

/**
 * @file mypopen.h
 * Betriebsysteme popen Modul.
 * Beispiel 1.2
 *
 * @author Thomas M. Galla <galla@technikum-wien.at>
 * @date 2006/03/18
### FP BP: Hmm ......
 *
 * @version $Revision: 481 $
 *
 * @todo Nothing to do. - Everything perfect! ;-)
 *
 * URL: $HeadURL: https://svn.petrovitsch.priv.at/ICSS-BES/trunk/2015/src/mypopen/mypopen.h $
 *
 * Last Modified: $Author: bernd $
 */

#ifndef _MYPOPEN_H_
#define _MYPOPEN_H_

/*
 * -------------------------------------------------------------- includes --
 */

/*
 * --------------------------------------------------------------- defines --
 */

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
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
 * \param cmd command to execute in the context of the child process
 * \param type "r" or "w" depending on whether the parent process
 *        intends to read from or write to the pipe.
 *
 * \return file pointer to the parent's pipe end or NULL in case of an error
 *
 */
/*
* ### FB: Name der Variablen ist zwischen H-File und C-Source Unterschiedlich --> Kein Fehler aber unschoen
*         FILE *mypopen(const char *command, const char *modus)
*/                  
extern FILE *mypopen(
    const char *cmd,
    const char *type
    );


/**
 * \brief pclose clone
 *
 * This function closes the pipe, waits for the child process to exit
 * and retrieves and returns the exit status of the child.
 *
 * \param fp file pointer to parent's pipe end
 *
 * \return exit status of child process or -1 in case of an error
 *
 */
/*
* ### FB: Name der Variablen ist zwischen H-File und C-Source Unterschiedlich --> Kein Fehler aber unschoen
*              int mypclose(FILE *fop)
*/                  
extern int mypclose(
    FILE *fp
    );

#endif /* _MYPOPEN_H_ */

/*
 * =================================================================== eof ==
 */