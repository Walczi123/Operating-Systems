#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
                                     exit(EXIT_FAILURE))

volatile sig_atomic_t children_left = 0;

void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_sigaction = f;
        act.sa_flags=SA_SIGINFO;
        if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}
void mq_handler(int sig, siginfo_t *info, void *p) {
        mqd_t *pin;
        uint8_t ni;
        unsigned msg_prio;

        pin = (mqd_t *)info->si_value.sival_ptr;

        static struct sigevent not;
        not.sigev_notify=SIGEV_SIGNAL;
        not.sigev_signo=SIGRTMIN;
        not.sigev_value.sival_ptr=pin;
        if(mq_notify(*pin, &not)<0)ERR("mq_notify");

        for(;;){
                if(mq_receive(*pin,(char*)&ni,1,&msg_prio)<1) {
                        if(errno==EAGAIN) break;
                        else ERR("mq_receive");
                }
                if(0==msg_prio) printf("MQ: got timeout from %d.\n",ni);
                else printf("MQ:%d is a bingo number!\n",ni);
        }

}
void sigchld_handler(int sig, siginfo_t *s, void *p) {
        pid_t pid;
        for(;;){
                pid=waitpid(0, NULL, WNOHANG);
                if(pid==0) return;
                if(pid<=0) {
                        if(errno==ECHILD) return;
                        ERR("waitpid");
                }
                children_left--;
        }
}

char* name(int pid, char* a){
        char *str = malloc(sizeof(char)* 12),tmp[11];
        str[0]='/';
        sprintf(tmp, "%d", pid);
        strcat(str,tmp);
        strcat(str,a);
        printf("%s\n",str);
        return str;
}

void server_work(mqd_t mq_s,mqd_t mq_d,mqd_t mq_m){
        int8_t s;
        while(1){
                if(TEMP_FAILURE_RETRY(mq_receive(mq_s,(char*)&s,1,NULL))<1)ERR("mq_receive");
                printf("[%d] Received %d\n",getpid(),s);
                if(TEMP_FAILURE_RETRY(mq_send(mq_s,(const char*)&s,1,1)))ERR("mq_send");
                printf("Server_work Quit\n");
                break;
        }
        return;
}

void make_queues(int n){

}

int main(int argc, char** argv) {
        mqd_t mq_s,mq_d,mq_m;
        struct mq_attr attr;
        attr.mq_maxmsg=10;
        attr.mq_msgsize=1;
        char *nm=name(getpid(),"_m"),*ns=name(getpid(),"_s"),*nd=name(getpid(),"_d");
        if((mq_s=TEMP_FAILURE_RETRY(mq_open(ns, O_RDWR|O_NONBLOCK | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq_s open in");
        if((mq_d=TEMP_FAILURE_RETRY(mq_open(nm, O_RDWR|O_NONBLOCK | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq_m open in");
        if((mq_m=TEMP_FAILURE_RETRY(mq_open(nd, O_RDWR|O_NONBLOCK | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq_d open in");
        
        sethandler(sigchld_handler,SIGCHLD);
        sethandler(mq_handler,SIGRTMIN);

        static struct sigevent not;
        not.sigev_notify=SIGEV_SIGNAL;
        not.sigev_signo=SIGRTMIN;
        not.sigev_value.sival_ptr=&mq_s;
        if(mq_notify(mq_s, &not)<0)ERR("mq_notify");


        server_work(mq_s,mq_d,mq_m);

        mq_close(mq_s);
        mq_close(mq_d);
        mq_close(mq_m);
        if(mq_unlink(ns))ERR("mq unlink");
        if(mq_unlink(nm))ERR("mq unlink");
        if(mq_unlink(nd))ERR("mq unlink");
        free(ns);
        free(nm);
        free(nd);
        return EXIT_SUCCESS;
}