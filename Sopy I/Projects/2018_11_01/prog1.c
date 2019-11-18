#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 20

int main(int argc, char** argv) {
	int x,i;
	char *env = getenv("TIMES");
	if(env) x = atoi(env);
	else x = 1;
	char name[MAX_LINE+2];
	
	while(fgets(name,MAX_LINE+2,stdin)!=NULL)
		for(i=0;i<x;i++)
			printf("Hello %s",name);

	if(putenv("RESULT=Done")!=0) {
		fprintf(stderr,"putenv failed");
		return EXIT_FAILURE;
	}
	printf("%s\n",getenv("RESULT"));
	if(system("env|grep RESULT")!=0) 
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

/*extern char **environ;

int main(int argc, char** argv) {
	int index = 0;
	while (environ[index])
		printf("%s\n", environ[index++]);
	return EXIT_SUCCESS;
}*/

/*void usage(char* pname)
{
	fprintf(stderr,"USAGE:%s ([-t x] -n Name) ... \n",pname);
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
	int c,i;
	int x = 1;
	while ((c = getopt (argc, argv, "t:n:")) != -1)
		switch (c)
		{
			case 't':
				x=atoi(optarg);
				break;
			case 'n':
				for(i=0;i<x;i++)
					printf("Hello %s\n",optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
		}
	if(argc>optind) usage(argv[0]);
	return EXIT_SUCCESS;
}*/

/*int main(int agrc, char** agrv)
{
	char name[30],loop[3];
	scanf("%29s",name);
	scanf("%2s",loop);
	int l=atoi(loop);
	for(int i=0;i<l;i++)
	{
		printf("%i.%s\n",i+1,name);
	}
	return EXIT_SUCCESS;
}*/

/*
int main(int argc, char** argv)
{
	int i;
	for(i=0;i<argc;i++)
		printf("%s\n",argv[i]);
}*/

/*int main(int argc, char** argv)
{
	char line[22];
	fgets(line,21,stdin);
	printf("\nYou write : %s\n",line);
	return EXIT_SUCCESS;
}*/

/*int main(int argc, char** argv)
{
	perror("Error bo tak\n");
	return EXIT_SUCCESS;
}*/

/*#define ERR(source) (perror(source),\
				fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
	 			exit(EXIT_FAILURE))*/

/*
int main(int argc, char** argv) {
	char name[22];
	scanf("%21s",name);
  if(strlen(name)>20) ERR("Name too long");
	printf("Hello %s\n",name);
	return EXIT_SUCCESS;
}*/