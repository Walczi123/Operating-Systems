#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

int main(int argc, char** argv)
{
    DIR *dirp;
	struct dirent *dp;
    struct stat filestat;
    long int sum=0;
    if (NULL == (dirp = opendir(argv[1]))) ERR("opendir");
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
    return EXIT_SUCCESS;
}