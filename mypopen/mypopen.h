#ifdef _MYPOPEN_H_

#define _MYPOPEN_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

FILE *mypopen(const char *command, const char *type);
int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */