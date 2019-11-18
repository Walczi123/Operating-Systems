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

char* name(int pid, char* a){
        char *str = malloc(sizeof(char)* 12),tmp[11];
        str[0]='/';
        sprintf(tmp, "%d", pid);
        strcat(str,tmp);
        strcat(str,a);
        printf("%s\n",str);
        return str;
}

int main(int argc, char** argv) {
        mqd_t mq;
        struct mq_attr attr;
        attr.mq_maxmsg=10;
        attr.mq_msgsize=1;
        char *n=name(getpid(),"");
        if((mq=TEMP_FAILURE_RETRY(mq_open(n, O_RDWR|O_NONBLOCK | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq open in");
        sleep(1);
        mq_close(mq);
        free(n);
        return EXIT_SUCCESS;
}