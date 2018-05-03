
/**
 * @file mypopen.c
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

#include "mypopen.h"

/**
 * a global variable containing the process id returned by fork
 */
static pid_t pid = -1;

/**
 * a global variable containing the pipe stream pointer initiated by mypopen
 */

 /* 
  * ### FB: < Implementierung als globale variable nicht nötig. Filepointer kann direkt aus Funktion mypopen retourniert werden ! 
  *           Hier kann myopen nur einmalig also aus einem Thread im Prozeß der die Library geladen hat aufgerufen werden. Bei Rückgabe
  *           des Filepointers aus mypopen kann man mypopen aus mehreren Threads gleichzeitig aufrufen>
  * ### FB BP: Wie soll dann der Check in mypclose() funktionieren können?
  */  
static FILE *global_stream = NULL;

/**
 * @brief initiate a pipe stream to or from a process
 *
 * @param command the command to be executed
 * @param type the I/O mode (r/w)
 *
 * @returns a file pointer or NULL in case of error
 */
FILE *mypopen(const char *command, const char *type) {
  int pipe_ends[2];
  int parent, child;

  /* check if already open */
  /* ### FB: Siehe Kommentar oben. global_stream als lokale Variable anstatt globaler Variable */
  if (global_stream != NULL) {
    errno = EAGAIN;
    return NULL;
  }

  /* check the command input */
  if (command == NULL) {
    errno = EINVAL;
    return NULL;
  }

  /* check the type inout length */
  if (type[1] != '\0') {
    errno = EINVAL;
    return NULL;
  }

  /* process the type input */
  switch (type[0]) {
  case 'r':
    parent = STDIN_FILENO;
    child = STDOUT_FILENO;
    break;
  case 'w':
    parent = STDOUT_FILENO;
    child = STDIN_FILENO;
    break;
  default:
    errno = EINVAL;
    return NULL;
  }

  /* create a pipe */
  if (pipe(pipe_ends) == -1) {
    /* errno is set by pipe */
    return NULL;
  }

  /* create a child process */
  switch (pid = fork()) {
  /* error */
  case -1:
    close(pipe_ends[parent]);
    close(pipe_ends[child]);
    /* errno is set by fork */
    return NULL;
  /* child */
  case 0:
    close(pipe_ends[parent]);
// ### FB BP: Sehr gut! Check, daß wir nicht stdout/stdin schließen! [+2]
    if (pipe_ends[child] != child) {
      if (dup2(pipe_ends[child], child) == -1) {
        close(pipe_ends[child]);
        _exit(1); /* catchall for general errors */
      }
      close(pipe_ends[child]);
    }
	/* ### FB: Vielleicht pingelig aber (void) execl("/bin/sh", "sh", "-c", command, NULL); wäre korrekter */
// ### FB BP: Das müßte 100% korrekterweise "(char*)NULL" sein, wie auch in `man 2 execl` nachzulesen ist.
//            Ich zieh' keine Punkte ab, weil ich nicht in beiden Verbänden das explizit erzählt hab. [-0]
    execl("/bin/sh", "sh", "-c", command, NULL);
    /* reached only if execl failed */
    _exit(127); /* command not found */
  /* parent */
  default:
    close(pipe_ends[child]);
    if ((global_stream = fdopen(pipe_ends[parent], type)) == NULL) {
      close(pipe_ends[parent]);
// ### FB BP: Gut, daß das nicht mehr benutzte Pipe-Ende auch noch geschlossen wird.
//            Und was ist mit dem Kind-Prozeß - da wird keiner mypclose() aufrufen ..... [-0]
      /* errno is set by fdopen */
      return NULL;
    }
  }

  return global_stream;
}

/**
 * @brief close a pipe stream to or from a process
 *
 * @param stream the stream to be closed
 *
 * @returns the exit status of the process or -1 in case of error
 */
int mypclose(FILE *stream) {
  pid_t wait_pid;
  int status;

  /* check if mypopen was previously run */
  if (global_stream == NULL) {
    errno = ECHILD;
    return -1;
  }

  /* check if we are closing the correct stream */
  if (global_stream != stream) {
    errno = EINVAL;
    return -1;
  }

  /* close the stream */
  if (fclose(stream) == EOF) {
    pid = -1;
    global_stream = NULL;
    /* errno is set by fclose */
    return -1;
  }

  /* wait for the child process to terminate */
  while ((wait_pid = waitpid(pid, &status, 0)) != pid) {
    if (wait_pid == -1) {
      if (errno == EINTR) {
        continue;
      }
      /* reached only in case of error */
      pid = -1;
      global_stream = NULL;
	  
      /* errno is set by waitpid */
      return -1;
    }
  }

  /* reset the global variables */
  pid = -1;
  global_stream = NULL;

  /* check if the child process terminated normally */
  if (WIFEXITED(status) != 0) {
    return WEXITSTATUS(status);
  }

  /* reached only if the process did not terminate normally */
  errno = ECHILD;
  return -1;
}
