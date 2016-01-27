//
//  dirread.cpp
//  
//
//  Created by Cannon Collins on 1/26/16.
//
//
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
int main()
{
    int len;
    DIR *dirp;
    struct dirent *dp;
    
    dirp = opendir(".");
    while ((dp = readdir(dirp)) != NULL) {
        //print to a string that generates the html
        printf("name %s\n", dp->d_name);
    }
    (void)closedir(dirp);
    return 0;
}