/**
 * @file mypopen.h
 * Betriebssysteme mypopen file
 * Beispiel 2
 *
 * @author Rostyslav Yavorskyi <rostyslav.yavorskyi@technikum-wien.at>
 * @author Alexander Manafi <alexander.manafi@technikum-wien.at>
 * @author Akhmed Khalikov <akhmed.khalikov@technikum-wien.at>
 * @date 2016/04/22
 *
 * @version 1.0.0
 */

#ifndef _MYPOPEN_H_
#define _MYPOPEN_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

FILE *mypopen(const char *command, const char *type);
int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */
