#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     exit(EXIT_FAILURE))


void create_children_and_pipes(char* com,int R) {
	int tmpfd[2];
    if(pipe(tmpfd)) ERR("pipe");
    switch (fork()) {
        case 0:
            if(TEMP_FAILURE_RETRY(close(tmpfd[0]))) ERR("close");
            printf("CHILD %d close descriptor %i\n",getpid(),tmpfd[0]);
            if(TEMP_FAILURE_RETRY(close(tmpfd[1]))) ERR("close");
            printf("CHILD %d close descriptor %i\n",getpid(),tmpfd[1]);           
            exit(EXIT_SUCCESS);

        case -1: ERR("Fork:");
    }
}

void usage(char * name){
	fprintf(stderr,"USAGE: %s n\n",name);
	fprintf(stderr,"0<n<=10 - number of children\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int R[2];
    char* com;
	if(2!=argc) usage(argv[0]);
    if(NULL==(com=(char*)malloc(sizeof(argv[1])))) ERR("malloc");
	if(pipe(R)) ERR("pipe");
    create_children_and_pipes(com,R[1]);
    if(TEMP_FAILURE_RETRY(close(R[0]))) ERR("close");
    printf("PARENT %d close descriptor %i\n",getpid(),R[0]);
    if(TEMP_FAILURE_RETRY(close(R[1]))) ERR("close");
    printf("PARENT %d close descriptor %i\n",getpid(),R[1]);
	
    free(com);
	return EXIT_SUCCESS;
}
