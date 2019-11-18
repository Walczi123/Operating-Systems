#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ftw.h>
#define MAXFD 20

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))


int files=0;

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
	switch (type){
		case FTW_F: files++; break;
	}
	return 0;
}

int main(int argc, char** argv) {
	int i;
	for(i=1;i<argc;i++){
		if(nftw(argv[i],walk,MAXFD,FTW_PHYS)==0)
			printf("%s:\nfiles:%d\n",argv[i],files);
		else printf("%s: brak dostępu\n",argv[i]);
		files=0;
	}
	return EXIT_SUCCESS;
}