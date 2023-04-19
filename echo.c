#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    if(argv[1] == NULL){
       printf("\n"); 
    } 
    else{
        printf("%s\n", argv[1]);
    }
    return 1;
}
