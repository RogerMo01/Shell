#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

int main(char* args)
{
    char currentPath[56];
    // Set current directory
    if(getcwd(currentPath, sizeof(currentPath)) == NULL) perror("error: Cannot access to current dir\n"); 
    printf("%s\n", currentPath);
    return 0;
}
