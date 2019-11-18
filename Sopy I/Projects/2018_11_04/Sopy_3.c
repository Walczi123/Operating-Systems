#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#define MAX_PATH 100

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

void scan_dir(void){
    DIR *dirp;
	struct dirent *dp;
    struct stat filestat;
    long int sum=0;
    if (NULL == (dirp = opendir("."))) ERR("opendir");
	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
            if (lstat(dp->d_name, &filestat)) ERR("lstat");
            fprintf(stdout,"%s: ",dp->d_name);
            fprintf(stdout,"%ld\n",filestat.st_size);
            sum+=filestat.st_size;
		}
	} while (dp != NULL);
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
    fprintf(stdout,"\nSum:    %ld\n",sum);
    return;
}

int main(int argc, char** argv){
	int i;
	char path[MAX_PATH];
	if(getcwd(path, MAX_PATH)==NULL) ERR("getcwd");
	for(i=1;i<argc;i++){
		if(chdir(argv[i]))ERR("chdir");
		printf("%s:\n",argv[i]);
		scan_dir();
		if(chdir(path))ERR("chdir");
	}
	return EXIT_SUCCESS;
}
