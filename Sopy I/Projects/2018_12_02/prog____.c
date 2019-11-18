#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define DEFAULT_A 100
#define DEFAULT_Q 3
#define DELETED_ITEM -1
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))
#define NEXT_INT(start,max) start+((int) rand()%(max))
typedef struct argsSignalHandler {
	pthread_t tid;
	int *pArrayCount;
	pthread_mutex_t *pmxASize;
	int *array;
	pthread_mutex_t *pmxArray;
    int* Sum;
	pthread_mutex_t *pmxSum;
	sigset_t *pMask;
	bool *pQuitFlag;
	pthread_mutex_t *pmxQuitFlag;
} argsSignalHandler_t;

void thread_fun(argsSignalHandler_t* the);
void ReadArguments(int, char**, int*, int*);
void removeItem(int *array, int *arrayCount, int index);
void printArray(int *array, int arraySize);
void* signal_handling(void*);

int main(int argc, char** argv) {
	int a,q,i;
	ReadArguments(argc, argv, &a, &q);
    pthread_mutex_t mxQuitFlag = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mxArray = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mxASize = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxSum = PTHREAD_MUTEX_INITIALIZER;
    bool *quitFlag;
    if(NULL==(quitFlag= (bool*) malloc(sizeof(bool))))ERR("Malloc error for flag!");
    *quitFlag=false;
    int *array,*arraySize,*sum;
    if(NULL==(arraySize= (int*) malloc(sizeof(int))))ERR("Malloc error for arraysize!");
    if(NULL==(sum = (int*) malloc(sizeof(int))))ERR("Malloc error for sum!");
    *sum=0;
	printf("%d\n",a);
    *arraySize=a;
	
	if(NULL==(array = (int*) malloc(sizeof(int) * a)))ERR("Malloc error for array!");
	for (int i =0; i < a; i++) array[i] = i + 1;
	sigset_t oldMask, newMask;
	sigemptyset(&newMask);
	sigaddset(&newMask, SIGINT);
	sigaddset(&newMask, SIGQUIT);
	if (pthread_sigmask(SIG_BLOCK, &newMask, &oldMask)) ERR("SIG_BLOCK error");
	argsSignalHandler_t* args;
	
    if(NULL==(args = (argsSignalHandler_t*) malloc(sizeof(argsSignalHandler_t) * q)))ERR("Malloc error for array!");
	for(i=0;i<q;i++){
        args[i].pArrayCount = arraySize;
		//printf("args[%d] have %d\n",i,*args[i].pArrayCount);
		args[i].pmxASize=&mxASize;
        args[i].array = array;
        args[i].pmxArray = &mxArray;
        args[i].pMask = &newMask;
        args[i].pQuitFlag = quitFlag;
        args[i].pmxQuitFlag = &mxQuitFlag;
        args[i].Sum=sum;
        args[i].pmxSum=&mxSum;
    }
	for(i=0;i<q;i++)
        if(pthread_create(&args[i].tid, NULL, signal_handling, &args))ERR("Couldn't create signal handling thread!");
	while (true) {
		pthread_mutex_lock(&mxQuitFlag);
		if (*quitFlag == true) {
			pthread_mutex_unlock(&mxQuitFlag);
			break;
		}
	}
	for(i=0;i<q;i++)
	   if(pthread_join(args[i].tid, NULL)) ERR("Can't join with 'signal handling' thread");

	free(array);
	free(sum);
	free(arraySize);
	free(quitFlag);
    free(args);
	printf("Wyjscie\n");
	if (pthread_sigmask(SIG_UNBLOCK, &newMask, &oldMask)) ERR("SIG_BLOCK error");
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

void printArray(int* array, int arraySize) {
	printf("[");
	for (int i =0; i < arraySize; i++)
			printf(" %d", array[i]);
	printf(" ]\n");
}

void* signal_handling(void* voidArgs) {
	argsSignalHandler_t* args = voidArgs;
	int signo;
    int l1,l2;
	srand(time(NULL));
	for (;;) {
		pthread_mutex_lock(args->pmxASize);
        //printArray(array, arraySize);
		//printf("Tutaj bez błedu %d\n",*args->pArrayCount);
		l1 = NEXT_INT(0,*args->pArrayCount);
        l2 = NEXT_INT(0,*args->pArrayCount);
		
        if(l1==l2){
            l2=(NEXT_INT(0,*args->pArrayCount)+l2) % *args->pArrayCount;
        }
		pthread_mutex_unlock(args->pmxASize);          
        pthread_mutex_lock(args->pmxSum);
        *args->Sum=*args->Sum+(args->array[l1]+args->array[l2]);
        if(args->array[l1]!=-1)args->array[l1]=-1;
        if(args->array[l2]!=-1)args->array[l2]=-1;
        pthread_mutex_unlock(args->pmxArray);
        pthread_mutex_unlock(args->pmxSum);
        
        
        if(sigwait(args->pMask, &signo)) ERR("sigwait failed.");
		printf("%d\n",signo);
		switch (signo) {
			case SIGINT:
                pthread_mutex_lock(args->pmxSum);
				printf("Sum=%d\n", *args->Sum);
                pthread_mutex_unlock(args->pmxSum);
				break;
			case SIGQUIT:
				pthread_mutex_lock(args->pmxQuitFlag);
				*args->pQuitFlag = true;
				pthread_mutex_unlock(args->pmxQuitFlag);
				return NULL;
			default:
				printf("unexpected signal %d\n", signo);
				exit(1);
		sleep(1);
		}
	}
	return NULL;
}
