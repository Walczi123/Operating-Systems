#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>


#define DEFAULT_A 100
#define DEFAULT_Q 3
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))
#define NEXT_INT(start,max) start+((int) rand()%max)
void ReadArguments(int, char**, int*, int*);
void* thread_function(void*);
void msleep(unsigned int);
typedef struct timespec timespec_t;
typedef struct qthread {
	pthread_t tid;
	int *Vec;
	unsigned int MySize;
	pthread_mutex_t *pmxVec;
	pthread_mutex_t *pmxMySize;
}qthread_t;

int main(int argc, char** argv) {
	int a,q;
	ReadArguments(argc, argv, &a, &q);
	printf("Here's a=%d and q=%d\n",a,q);
	int *Vector=(int*) malloc(sizeof(int) * a),i;
	if (Vector == NULL) 
		ERR("Failed to allocate memory for 'students list'!");
	for(i=0;i<a;i++){
		Vector[i]=i+1;
	}
	pthread_mutex_t mxVec = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mxMySize = PTHREAD_MUTEX_INITIALIZER;
	qthread_t* thread = (qthread_t*) malloc(sizeof(qthread_t) * q);	
	if (thread == NULL) ERR("Malloc error for estimation arguments!");
	for (i = 0; i < q; i++) {	
		thread->Vec=Vector;
		thread->pmxVec=&mxVec;
		thread->MySize=q;
		thread->pmxVec=&mxMySize;
	}
	for (int i = 0; i < q; i++) {
		int err = pthread_create(&(thread[i].tid), NULL, thread_function, &thread[i]);
		if (err != 0) ERR("Couldn't create thread");
	}
	int *tmp ;
	for (int i = 0; i < q; i++) {
		int err = pthread_join(thread[i].tid,(void*)&tmp);
		if (err != 0) ERR("Can't join with a thread");
		free(tmp);
	}
	exit(EXIT_SUCCESS);
}

void ReadArguments(int argc, char** argv, int *a, int *q) {
	*a= DEFAULT_A;
	*q=DEFAULT_Q;
	if (argc >= 2) {
		*a = atoi(argv[1]);
		if (*a <= 0) {
			printf("Invalid value for 'balls count'");
			exit(EXIT_FAILURE);
		}
	}
	if (argc >= 3) {
		*q = atoi(argv[2]);
		if (*q <= 0) {
			printf("Invalid value for 'throwers count'");
			exit(EXIT_FAILURE);
		}
	}
}

void* thread_function(void *voidPtr) {
	qthread_t* args = voidPtr;
	//printf("Zyje %ld\n",args->tid);
	srand(time(NULL));
	int l1,l2,*sum;
	if(NULL==(sum=malloc(sizeof(int)))) ERR("malloc");
	*sum=0;

	while(1){
		l1 = NEXT_INT(0,args->MySize);
		l2 = NEXT_INT(0,args->MySize);
		pthread_mutex_lock(&args->pmxVec);	
		pthread_mutex_lock(&args->pmxMySize);		
		if(l1==l2){
			l2=(NEXT_INT(0,args->MySize)+l2) % args->MySize;
		}
		pthread_mutex_unlock(&args->pmxMySize);
		*sum=*sum+(args->Vec[l1]+args->Vec[l2]);
		if(args->Vec[l1]!=-1)args->Vec[l1]=-1;
		if(args->Vec[l2]!=-1)args->Vec[l2]=-1;
		pthread_mutex_unlock(&args->pmxVec);
		msleep(1000);
	}
	return sum;
}

void msleep(unsigned int milisec) {
    time_t sec= (int)(milisec/1000);
    milisec = milisec - (sec*1000);
    timespec_t req= {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if(nanosleep(&req,&req)) ERR("nanosleep");
}