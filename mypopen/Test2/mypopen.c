#include <unistd.h>
#include <stdio.h>
#include "mypopen.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


/* TEST STATUS
./popentest: Test "mypopentest00" passed
./popentest: Test "mypopentest01" passed
./popentest: Test "mypopentest02" passed
./popentest: Test "mypopentest03" passed
./popentest: Test "mypopentest04" passed
./popentest: Test "mypopentest05" passed
./popentest: Test "mypopentest06" passed
./popentest: Test "mypopentest07" passed
./popentest: Test "mypopentest08" passed
./popentest: Test "mypopentest09" passed
./popentest: Test "mypopentest10" passed
./popentest: Test "mypopentest11" passed
./popentest: Test "mypopentest12" passed
./popentest: Test "mypopentest13" passed
./popentest: Test "mypopentest14" passed
./popentest: Test "mypopentest15" passed
./popentest: Test "mypopentest16" passed
./popentest: Test "mypopentest17" passed
./popentest: Test "mypopentest18" passed
./popentest: Test "mypopentest19" passed
./popentest: Test "mypopentest20" passed
./popentest: Test "mypopentest21" passed
./popentest: Test "mypopentest22" passed
./popentest: Test "mypopentest23" passed
./popentest: Test "mypopentest24" failed at line 2354 - This is not tolerated
./popentest: Test "mypopentest25" failed at line 2466 - This is not tolerated
./popentest: Test "mypopentest26" passed
./popentest: Test "mypopentest27" failed at line 2492 - This is not tolerated
./popentest: Test "mypopentest27" passed
./popentest: Test "mypopentest28" failed at line 2265 - This is not tolerated
./popentest: Test "mypopentest28" passed
./popentest: Test "mypopentest29" passed
./popentest: Test "mypopentest30" passed
*/



static pid_t pid = -1;
FILE *retpointer = NULL;
	
enum { PIPE_FD_READ, PIPE_FD_WRITE};
FILE *mypopen(const char *command, const char *type){
	
	
	if(type == NULL){
		errno = EINVAL;
		return NULL;
	}
	
	
	if((type[0] != 'r' && type[0] != 'w') || type[1] != '\0'){
		errno = EINVAL;
		return NULL;
	}	
	
	
	if(pid != -1){
		errno = EAGAIN;
		return NULL;
	}
	
	
	int pipefd[2];

	if (pipe(pipefd) == -1) {
		// erno already set by pipe
		return NULL;
	}

    pid = fork();
	
	if(pid == -1){
		close(pipefd[PIPE_FD_READ]);
        close(pipefd[PIPE_FD_WRITE]);
		errno = EAGAIN;
		return NULL;
	}
		
	if(pid == 0){
		//child
		if(type[0] == 'r') {
			
			int dupret = dup2(pipefd[PIPE_FD_WRITE], STDOUT_FILENO);
			if(dupret == -1){
				close(pipefd[PIPE_FD_WRITE]);
				exit(EXIT_FAILURE);
			}
		}
		if(type[0] == 'w'){
			int dupret = dup2(pipefd[PIPE_FD_READ] , STDIN_FILENO );
			if(dupret == -1){
				close(pipefd[PIPE_FD_READ]);
				exit(EXIT_FAILURE);
			}
			
		}
		
		close(pipefd[PIPE_FD_READ]);
		close(pipefd[PIPE_FD_WRITE]);
		
		execl("/bin/sh", "sh", "-c", command, (char*) NULL);
		//reaches only when execl fails
		exit(EXIT_FAILURE);
		
	}else{
		if(type[0] == 'r') {
			close(pipefd[PIPE_FD_WRITE]);
			retpointer =  fdopen(pipefd[PIPE_FD_READ], "r");
		}
		if(type[0] == 'w'){
			close(pipefd[PIPE_FD_READ]);
			retpointer =  fdopen(pipefd[PIPE_FD_WRITE], "w");
		}
	}

	if(retpointer == NULL){
		pid = -1;
		
		if(type[0] == 'r') close(pipefd[PIPE_FD_READ]);
		else if(type[0] == 'w') close(pipefd[PIPE_FD_WRITE]);
	
		else return NULL;
	}
	return retpointer;
	 
}

int mypclose(FILE *stream){
	
	if (pid == -1){
		errno = ECHILD;
		return -1;
	}
	
	if(retpointer != stream || retpointer == NULL){
		errno = EINVAL;
		return -1;		
	}
	
	if(fclose(retpointer) == -1){
		//reset vars.
		pid = -1;
		retpointer = NULL;
		
		return -1;
	}
	
	int status;
	pid_t wait_pid;
	do{
		wait_pid = waitpid(pid, &status,0);
	}while(wait_pid == -1 && errno == EINTR);
    
	
	//reset Vars.
	pid = -1;
	
    if (wait_pid == -1){
        errno = ECHILD;
        return -1;
    }

    if (WIFEXITED(status)) {
       return WEXITSTATUS(status);
	}else{
	errno = ECHILD;
    return -1;		   
	}
}