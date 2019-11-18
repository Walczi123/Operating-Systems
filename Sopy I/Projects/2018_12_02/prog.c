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
#define NEXT_INT(start,max) start+((int) rand()%max)
typedef struct argsSignalHandler {
	pthread_t tid;
	int *pArrayCount;
	int *array;
	pthread_mutex_t *pmxArray;
    int* Sum;
	pthread_mutex_t *pmxSum;
	sigset_t *pMask;
	bool *pQuitFlag;
	pthread_mutex_t *pmxQuitFlag;
} argsSignalHandler_t;

void ReadArguments(int, char**, int*, int*);
void removeItem(int *array, int *arrayCount, int index);
void printArray(int *array, int arraySize);
void* signal_handling(void*);

int main(int argc, char** argv) {
	int a,q;
	ReadArguments(argc, argv, &a, &q);
    pthread_mutex_t mxQuitFlag = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t mxArray = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mxSum = PTHREAD_MUTEX_INITIALIZER;
    bool quitFlag = false;
    int *array,arraySize=a,*sum;
    if(NULL==(sum = (int*) malloc(sizeof(int))))ERR("Malloc error for sum!");
    *sum=0;
	if(NULL==(array = (int*) malloc(sizeof(int) * arraySize)))ERR("Malloc error for array!");
	for (int i =0; i < arraySize; i++) array[i] = i + 1;
	sigset_t oldMask, newMask;
	sigemptyset(&newMask);
	sigaddset(&newMask, SIGINT);
	sigaddset(&newMask, SIGQUIT);
	if (pthread_sigmask(SIG_BLOCK, &newMask, &oldMask)) ERR("SIG_BLOCK error");
	argsSignalHandler_t args;
    //if(NULL==(array = (argsSignalHandler_t*) malloc(sizeof(argsSignalHandler_t))))ERR("Malloc error for array!");
	args.pArrayCount = &arraySize;
	args.array = array;
	args.pmxArray = &mxArray;
	args.pMask = &newMask;
	args.pQuitFlag = &quitFlag;
	args.pmxQuitFlag = &mxQuitFlag;
	args.Sum=sum;
	args.pmxSum=&mxSum;
	if(pthread_create(&args.tid, NULL, signal_handling, &args))ERR("Couldn't create signal handling thread!");
	int l1,l2;
    while (true) {
		pthread_mutex_lock(&mxQuitFlag);
		if (quitFlag == true) {
			pthread_mutex_unlock(&mxQuitFlag);
			break;
		} else {
			pthread_mutex_unlock(&mxQuitFlag);
			pthread_mutex_lock(&mxArray);
			//printArray(array, arraySize);
            l1 = NEXT_INT(0,arraySize);
            l2 = NEXT_INT(0,arraySize);
            if(l1==l2){
                l2=(NEXT_INT(0,arraySize)+l2) % arraySize;
            }           
            pthread_mutex_lock(&mxSum);
            *sum=*sum+(array[l1]+array[l2]);
            if(array[l1]!=-1)array[l1]=-1;
            if(array[l2]!=-1)array[l2]=-1;
            pthread_mutex_unlock(&mxArray);
            pthread_mutex_unlock(&mxSum);
            sleep(1);	
		}
	}
	if(pthread_join(args.tid, NULL)) ERR("Can't join with 'signal handling' thread");
	free(array);
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
	srand(time(NULL));
	for (;;) {
		if(sigwait(args->pMask, &signo)) ERR("sigwait failed.");
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
		}
	}
	return NULL;
}
