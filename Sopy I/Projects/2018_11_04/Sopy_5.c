#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#define MAX_PATH 100

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__))

void scan_dir(long int limit,FILE* s){
    DIR *dirp;
	struct dirent *dp;
    struct stat filestat;
    long int sum=0;
    if (NULL == (dirp = opendir("."))) ERR("opendir");
	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
            if (lstat(dp->d_name, &filestat)) ERR("lstat");
            if(filestat.st_size>limit){sum+=filestat.st_size;}
		}
	} while (dp != NULL);
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
    fprintf(s,"Sum:    %ld\n\n",sum);
    return;
}

int main(int argc, char** argv){
	FILE* s;
	if((s=fopen("out.txt","w+"))==NULL)	ERR("fopen");
    int i;
    long int limit;
	char path[MAX_PATH];
	if(getcwd(path, MAX_PATH)==NULL) ERR("getcwd");
	for(i=1;i<argc;i++){
		if(chdir(argv[i])){ERR("chdir");i++;}
        else{
		fprintf(s,"%s:\n",argv[i++]);
		if((limit=strtol(argv[i],NULL,10)))scan_dir(limit,s);
        //fprintf(s,"%ld\n",limit);
		if(chdir(path))ERR("chdir");
        }
	}
    if(fclose(s))ERR("fclose");
	return EXIT_SUCCESS;
}
