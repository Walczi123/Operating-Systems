#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

char* itoa(int a){
    char* res="0";
    if(a==0) return res;
    int b=a,n=0,i=0;    
    while(b!=0){
        b=b/10;
        n++;
    }
    char *buf=malloc(n+1);
    res=malloc(n+1);
    buf[n]='\0';
    for(;i<n;i++){
        b=a%10;
        a=a/10;
        buf[i]=b+'0';
    }
    strcpy(res,buf);
    for(i=0;i<n;i++){
        buf[i]=res[n-1-i];
    }
    free(res);
    return buf;
}

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;
volatile sig_atomic_t last_signal = 0;


void sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) {
	sig_count++;
    last_signal = sig;
}

void sigchld_handler(int sig) {
	pid_t pid;
	for(;;){
		pid=waitpid(0, NULL, WNOHANG);
		if(pid==0) return;
		if(pid<=0) {
			if(errno==ECHILD) return;
			ERR("waitpid");
		}
	}
}

ssize_t bulk_write(int fd, char *buf, size_t count){
	ssize_t c;
	ssize_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(write(fd,buf,count));
		if(c<0) return c;
		buf+=c;
		len+=c;
		count-=c;
	}while(count>0);
	return len ;
}


void child_work(int n) {
    srand(time(NULL)*getpid());
	int s=10*1024+rand()%(100*1024-10*1024+1),out;
    char* p,*name=itoa(getpid());
    p=name;
    strcat(name,".txt");
    char *buf=malloc(s);
    if((out=TEMP_FAILURE_RETRY(open(name,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,0777)))<0)ERR("open");
    if((s=write(out,buf,s))<0) ERR("read");
    if(TEMP_FAILURE_RETRY(close(out)))ERR("close");
    free(p);
    printf("[%d] signals=%d\n",getpid(),sig_count);
}


void parent_work() {
	struct timespec t = {10, 0};
    int i=0;
    sethandler(sig_handler,SIGALRM);
    while(last_signal!=SIGALRM) {
            nanosleep(&t,NULL);
            if (kill(0, SIGUSR1)<0)ERR("kill");
            i++;
            if(i>1)break;
    }
    printf("[PARENT] Terminates \n");
}

void create_children(int argc,char** argv) {
	int n=argc;
    while (n-->0) {
		switch (fork()) {
			case 0: 
                //sethandler(sig_handler,SIGUSR1);
				child_work(atoi(argv[argc-n]));
				exit(EXIT_SUCCESS);
			case -1:perror("Fork:");
				exit(EXIT_FAILURE);
		}
	}
}

void usage(char *name){
	fprintf(stderr,"USAGE: %s ... \n",name);
	fprintf(stderr,"Type more params\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	if(argc<2) usage(argv[0]);
    parent_work();
    create_children(argc,argv); 
	while(wait(NULL)>0);
	return EXIT_SUCCESS;
}