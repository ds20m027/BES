#ifdef _MYPOPEN_H_

#define _MYPOPEN_H_

FILE *mypopen(const char *command, const char *type);
int mypclose(FILE *stream);

#endif /* _MYPOPEN_H_ */