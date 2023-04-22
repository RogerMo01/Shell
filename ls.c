#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

void ls();

int main(char* args)
{
    ls();
    return 0;
}


// ~~~~ LS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ls()
{
    // create DIR object
    DIR *d;
    struct dirent *dir_ent;
    char currentPath[56];
    
    // Set current directory
    if(getcwd(currentPath, sizeof(currentPath)) == NULL) { fprintf(stderr, "Error: cannot access to current dir\n"); return; }


    d = opendir(currentPath);

    if(d == NULL) { fprintf(stderr, "Error: cannot open directory '%s'\n", currentPath); return; }

    // Gets the max len of files names
    int largest = 0;
    while((dir_ent = readdir(d)) != NULL)
    {
        if(dir_ent->d_name[0] == '.') continue; // ignore dot start files
        int current = (int)strlen(dir_ent->d_name);
        if(current > largest) largest = current;
    }
    closedir(d);
    d = opendir(currentPath);
    

    // Print all files
    int lineMaxLen = 100;
    int countInLine = lineMaxLen/(largest+2);
    int countInCol = 0;

    while((dir_ent = readdir(d)) != NULL)
    {
        if(dir_ent->d_name[0] == '.') continue; // ignore dot start files
        
        if(countInCol == countInLine)
        {
            printf("\n");
            countInCol = 0;
        }

        int currentLen = (int)strlen(dir_ent->d_name);
        int r = largest+2-currentLen;
        // print dirname
        printf("%s", dir_ent->d_name);
        // print r spaces
        for (int i = 0; i < r; i++) printf(" ");

        countInCol++;
    }
    printf("\n");

    closedir(d);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~