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

/*
* -------------------------------------------------------------- includes --
*/
#include "myipc.h"
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
FILE* mypopen(const char* command, const char* type)
{

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: