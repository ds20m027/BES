/**
* @file sender.c
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
* \brief Startet die Ausfuehrung des Senders
*
* Datenaustausch mittels Shared Memory und Semaphoren
* per Ringpuffer zwischen Sender und Empfänger
*
* \param argc Anzahl der Command Line Argumente
* \param Command Line Argumente
*
* \return EXIT_SUCCESS wenn kein Fehler auftritt, ansonsten EXIT_FAILURE
*/
int main (int argc, char * argv[])
{
	return ipc_start(argc, argv, TYPE_SENDER);
}

	// =================================================================== eof ==

	// Local Variables:
	// mode: c
	// c-mode: k&r
	// c-basic-offset: 8
	// indent-tabs-mode: t
	// End: