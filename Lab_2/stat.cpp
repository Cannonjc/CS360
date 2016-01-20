#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

using namespace std;

main(int argc, char **argv)
{
	struct stat filestat;

	if(stat(argv[1], &filestat)) {
		cout <<"ERROR in stat\n";
	}
	if(S_ISREG(filestat.st_mode)) {
		cout << argv[1] << " is a regular file \n";
		cout << "file size = "<<filestat.st_size <<"\n";
        FILE fp = fopen(argv[1]);
        char *buff = (char *)malloc(filestat.st_size);
        fread(buff,filestat.st_size,1,fp);
        cout << "FILE " << endl
	}
	if(S_ISDIR(filestat.st_mode)) {
		cout << argv[1] << " is a directory \n";
        DIR *dirp;
        struct dirent *dp;
        
        dirp = opendir(argv[1]);
        while ((dp = readdir(dirp)) != NULL)
            cout << "name " << dp->d_name << endl;
            printf("name %s\n", dp->d_name);
        (void)closedir(dirp);
	}
	
}