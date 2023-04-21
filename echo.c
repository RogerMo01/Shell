#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    // argv[0] - Text
    // argv[1] - (-1, i>0) rdIn

    if(strcmp(argv[1], "-1") == 0)
    {
        if(argv[0] == NULL || strcmp(argv[0], ">") == 0 || strcmp(argv[0], ">>") == 0)
        {
            printf(" \n");
        }
        else
        {
            printf("%s\n", argv[0]);
        }
    }
    else
    {
        char str[64];
        fgets(str, 64, stdin);

        // Remove \n
        int pos = (int)strcspn(str, "\n");
        if(pos < strlen(str)) str[pos] = '\0';

        printf("%s\n", str);
    }

    return 1;
}
