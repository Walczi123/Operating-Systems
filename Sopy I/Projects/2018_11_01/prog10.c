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


int dirs=0,files=0,links=0,other=0;

int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{
	switch (type){
		case FTW_DNR:
		case FTW_D: dirs++; break;
		case FTW_F: files++; break;
		case FTW_SL: links++; break;
		default : other++; 
	}
	return 0;
}

int main(int argc, char** argv) {
	int i;
	for(i=1;i<argc;i++){
		if(nftw(argv[i],walk,MAXFD,FTW_PHYS)==0)
			printf("%s:\nfiles:%d\ndirs:%d\nlinks:%d\nother:%d\n",argv[i],files,dirs,links,other);
		else printf("%s: brak dostÄpu\n",argv[i]);
		dirs=files=links=other=0;
	}
	return EXIT_SUCCESS;
}

/*#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#define MAX_PATH 101

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

void scan_dir (){
	DIR *dirp;
	struct dirent *dp;
	struct stat filestat;
	int dirs=0,files=0,links=0,other=0;
	if (NULL == (dirp = opendir("."))) ERR("opendir");
	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			if (lstat(dp->d_name, &filestat)) ERR("lstat");
			if (S_ISDIR(filestat.st_mode)) dirs++;
			else if (S_ISREG(filestat.st_mode)) files++;
			else if (S_ISLNK(filestat.st_mode)) links++;
			else other++;

		}
	} while (dp != NULL);

	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
	printf("Files: %d, Dirs: %d, Links: %d, Other: %d\n",files,dirs,links,other);
}

int main(int argc, char** argv) {
	/*int i;
	char path[MAX_PATH];
	if(getcwd(path, MAX_PATH)==NULL) ERR("getcwd");
	for(i=1;i<argc;i++){
		if(chdir(argv[i]))ERR("chdir");
		printf("%s:\n",argv[i]);
		scan_dir();
		if(chdir(path))ERR("chdir");
	}
    int i;
    for(i=1;i<argc;i++) 
        printf(argv[i]);
	return EXIT_SUCCESS;
}*/