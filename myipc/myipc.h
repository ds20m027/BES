/**
* @file myipc.h
* Betriebssysteme Beispiel 3
* myipc - Datenaustausch mittels Shared Memory und Semaphoren 
* per Ringpuffer zwischen Sender und Empfänger
*
* @author Ralf Ziefuhs <ic17b065@technikum-wien.at>
* @author Clemens Fritzsche <ic17b087@technikum-wien.at>
*
* @date 2018/06/15
*
* @version 1.0
*
*/

#ifndef _MYIPC_H_

#define _MYIPC_H_

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
* \brief ipc
*
* This function
*
* \param 
* \param 
*
* \return 
*/
extern FILE *mypopen(const char *command, const char *type);

/**
* \brief ipc
*
*
* \param 
*
* \return 
*/
extern int mypclose(FILE *stream);

#endif /* _MYIPC_H_ */

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: