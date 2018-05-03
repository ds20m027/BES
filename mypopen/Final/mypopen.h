/**
* @file mypopen.h
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

#ifndef _MYPOPEN_H_

#define _MYPOPEN_H_

FILE *mypopen(const char *command, const char *type);
int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */
