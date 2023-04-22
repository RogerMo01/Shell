#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

bool isOperator(char* arg);

int main(int argc, char *argv[])
{
    char* txtDir = argv[0];
    char arg[10];

    if(argv[1] != NULL) strcpy(arg, argv[1]);
    strcat(txtDir, "help");
    
    FILE* file;

    if(arg == NULL || strcmp(arg, "") == 0 || strcmp(arg, " ") == 0 || isOperator(arg))
    {
        strcat(txtDir, ".txt");
    }
    else
    {
        strcat(txtDir, "_");
        strcat(txtDir, arg);
        strcat(txtDir, ".txt");
    }

    file = fopen(txtDir, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(size + 1);
    fread(content, size, 1, file);
    content[size] = '\0';

    fclose(file);

    for (int i = 0; i < size; i++)
    {
        printf("%c", content[i]);
        if(i == size-1 && content[i] != '\n') printf("\n"); 
    }

    return 1;
}

bool isOperator(char* arg)
{
    if(strcmp(arg, ">") == 0 || strcmp(arg, ">>") == 0 || strcmp(arg, "<") == 0 || strcmp(arg, "|") == 0) return true; else return false;
}