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
/*
* --------------------------------------------------------------- defines --
*/
#define TYPE_SENDER 1
#define TYPE_EMPFAENGER 2

/*
* -------------------------------------------------------------- typedefs --
*/
/*
* ------------------------------------------------------------- functions --
*/

/**
* \brief Funktion für die Ausfuehrung des Senders oder Empfaengers
*
* Datenaustausch mittels Shared Memory und Semaphoren 
* per Ringpuffer zwischen Sender und Empfänger
*
* \param argc Anzahl der Command Line Argumente
* \param Command Line Argumente
*
* \return EXIT_SUCCESS wenn kein Fehler auftritt, ansonsten EXIT_FAILURE
*/
extern int start_ipc (int argc, const char const * argv [], const int type);

#endif /* _MYIPC_H_ */

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: