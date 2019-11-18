//PART IV
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

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t sig_count = 0;

void sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) {
	sig_count++;;
}

void child_work(int m) {
	struct timespec t = {0, m*10000};
	sethandler(SIG_DFL,SIGUSR1);
	while(1){
		nanosleep(&t,NULL);
		if(kill(getppid(),SIGUSR1))ERR("kill");
	}
}


ssize_t bulk_read(int fd, char *buf, size_t count){
	ssize_t c;
	ssize_t len=0;
	do{
		c=TEMP_FAILURE_RETRY(read(fd,buf,count));
		if(c<0) return c;
		if(c==0) return len; //EOF
		buf+=c;
		len+=c;
		count-=c;S
	}while(count>0);
	return len ;
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

void parent_work(int b, int s, char *name) {
	int i,in,out;
	ssize_t count;
	char *buf=malloc(s);
	if(!buf) ERR("malloc");
	if((out=TEMP_FAILURE_RETRY(open(name,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,0777)))<0)ERR("open");
	if((in=TEMP_FAILURE_RETRY(open("/dev/urandom",O_RDONLY)))<0)ERR("open");
	for(i=0; i<b;i++){
		if((count=bulk_read(in,buf,s))<0) ERR("read");
		if((count=bulk_write(out,buf,count))<0) ERR("read");
		if(TEMP_FAILURE_RETRY(fprintf(stderr,"Block of %ld bytes transfered. Signals RX:%d\n",count,sig_count))<0)ERR("fprintf");;
	}
	if(TEMP_FAILURE_RETRY(close(in)))ERR("close");
	if(TEMP_FAILURE_RETRY(close(out)))ERR("close");
	free(buf);
	if(kill(0,SIGUSR1))ERR("kill");
}

void usage(char *name){
	fprintf(stderr,"USAGE: %s m b s \n",name);
	fprintf(stderr,"m - number of 1/1000 milliseconds between signals [1,999], i.e. one milisecond maximum\n");
	fprintf(stderr,"b - number of blocks [1,999]\n");
	fprintf(stderr,"s - size of of blocks [1,999] in MB\n");
	fprintf(stderr,"name of the output file\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int m,b,s;
	char *name;
	if(argc!=5) usage(argv[0]);
	m = atoi(argv[1]); b = atoi(argv[2]);  s = atoi(argv[3]); name=argv[4];
	if (m<=0||m>999||b<=0||b>999||s<=0||s>999)usage(argv[0]); 
	sethandler(sig_handler,SIGUSR1);
	pid_t pid;
	if((pid=fork())<0) ERR("fork");
	if(0==pid) child_work(m);
	else {
		parent_work(b,s*1024*1024,name);
		while(wait(NULL)>0);
	}
	return EXIT_SUCCESS;
}



//PART III
/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

void sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) {
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

void child_work(int m, int p) {
	int count=0;
	struct timespec t = {0, m*10000};
	while(1){
		for(int i =0; i<p; i++){
			nanosleep(&t,NULL);
			if(kill(getppid(),SIGUSR1))ERR("kill");
		}
		nanosleep(&t,NULL);
		if(kill(getppid(),SIGUSR2))ERR("kill");
		count++;
		printf("[%d] sent %d SIGUSR2\n",getpid(), count);

	}
}


void parent_work(sigset_t oldmask) {
	int count=0;
	while(1){
		last_signal=0;
		while(last_signal!=SIGUSR2)
			sigsuspend(&oldmask);
		count++;
		printf("[PARENT] received %d SIGUSR2\n", count);
		
	}
}

void usage(char *name){
	fprintf(stderr,"USAGE: %s m  p\n",name);
	fprintf(stderr,"m - number of 1/1000 milliseconds between signals [1,999], i.e. one milisecond maximum\n");
	fprintf(stderr,"p - after p SIGUSR1 send one SIGUSER2  [1,999]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int m,p;
	if(argc!=3) usage(argv[0]);
	m = atoi(argv[1]); p = atoi(argv[2]);
	if (m<=0 || m>999 || p<=0 || p>999)  usage(argv[0]); 
	sethandler(sigchld_handler,SIGCHLD);
	sethandler(sig_handler,SIGUSR1);
	sethandler(sig_handler,SIGUSR2);
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	pid_t pid;
	if((pid=fork())<0) ERR("fork");
	if(0==pid) child_work(m,p);
	else {
		parent_work(oldmask);
		while(wait(NULL)>0);
	}
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	return EXIT_SUCCESS;
}
*/
//PART II
/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;

void sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void sig_handler(int sig) {
	printf("[%d] received signal %d\n", getpid(), sig);
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

void child_work(int l) {
	int t,tt;
	srand(getpid());
	t = rand()%6+5; 
	while(l-- > 0){
		for(tt=t;tt>0;tt=sleep(tt));
		if (last_signal == SIGUSR1) printf("Success [%d]\n", getpid());
		else printf("Failed [%d]\n", getpid());
	}
	printf("[%d] Terminates \n",getpid());
}

void parent_work(int k, int p, int l) {
	struct timespec tk = {k, 0};
	struct timespec tp = {p, 0};
	sethandler(sig_handler,SIGALRM);
	alarm(l*10);
	while(last_signal!=SIGALRM) {
		nanosleep(&tk,NULL);
		if (kill(0, SIGUSR1)<0)ERR("kill");
		nanosleep(&tp,NULL);
		if (kill(0, SIGUSR2)<0)ERR("kill");
	}
	printf("[PARENT] Terminates \n");
}

void create_children(int n, int l) {
	while (n-->0) {
		switch (fork()) {
			case 0: sethandler(sig_handler,SIGUSR1);
				sethandler(sig_handler,SIGUSR2);
				child_work(l);
				exit(EXIT_SUCCESS);
			case -1:perror("Fork:");
				exit(EXIT_FAILURE);
		}
	}
}

void usage(void){
	fprintf(stderr,"USAGE: signals n k p l\n");
	fprintf(stderr,"n - number of children\n");
	fprintf(stderr,"k - Interval before SIGUSR1\n");
	fprintf(stderr,"p - Interval before SIGUSR2\n");
	fprintf(stderr,"l - lifetime of child in cycles\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n, k, p, l;
	if(argc!=5) usage();
	n = atoi(argv[1]); k = atoi(argv[2]); p = atoi(argv[3]); l = atoi(argv[4]);
	if (n<=0 || k<=0 || p<=0 || l<=0)  usage(); 
	sethandler(sigchld_handler,SIGCHLD);
	sethandler(SIG_IGN,SIGUSR1);
	sethandler(SIG_IGN,SIGUSR2);
	create_children(n, l);
	parent_work(k, p, l);
	while(wait(NULL)>0);
	return EXIT_SUCCESS;
}*/

//PART I
/*
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))


void child_work(int i) {
        srand(time(NULL)*getpid());     
        int t=2+rand()%(10-5+1);
        sleep(t);
        printf("PROCESS with pid %d terminates\n",getpid());
}

void create_children(int n) {
        pid_t s;
        for (n--;n>=0;n--) {
                if((s=fork())<0) ERR("Fork:");
                if(!s) {
                        child_work(n);
                        exit(EXIT_SUCCESS);
                }
        }
}

void usage(char *name){
        fprintf(stderr,"USAGE: %s 0<n\n",name);
        exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n;
	if(argc<2)  usage(argv[0]);
	n=atoi(argv[1]);
	if(n<=0)  usage(argv[0]);
	create_children(n);
	while(n>0){
		sleep(3);
		pid_t pid;
		for(;;){
			pid=waitpid(0, NULL, WNOHANG);
			if(pid>0) n--;
			if(0==pid) break;
			if(0>=pid) {
				if(ECHILD==errno) break;
				ERR("waitpid:");
			}
		}
		printf("PARENT: %d processes remain\n",n);
	}
	return EXIT_SUCCESS;
}
*/
