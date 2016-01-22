#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char **argv)
{
	struct stat filestat;

	if(stat(argv[1], &filestat)) {
		printf("ERROR in stat\n");
	}
	if(S_ISREG(filestat.st_mode)) {
		cout << argv[1] << " is a regular file \n";
		cout << "file size = "<<filestat.st_size <<"\n";
        FILE *fp = fopen(argv[1],"r");
        char *buff = (char *)malloc(filestat.st_size);
        fread(buff,filestat.st_size,1,fp);
        printf("Got\n%s\n", buff);
	}
	if(S_ISDIR(filestat.st_mode)) {
		cout << argv[1] << " is a directory \n";
        DIR *dirp;
        struct dirent *dp;
        
        dirp = opendir(argv[1]);
        while ((dp = readdir(dirp)) != NULL)
            printf("name %s\n", dp->d_name);
        (void)closedir(dirp);
	}
	
}