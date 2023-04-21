#include <stdio.h>
#include <string.h>

void reverse_string(char *str);

void reverse_string(char *str) {
    int len = strlen(str);
    int i, j;
    char temp;
    for (i = 0, j = len - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

int main(int argc, char *argv[]) 
{
    // argv[0] = Text
    // argv[1] = rdIn
    // argv[2] = isPipe

    char str[56];
    if(strcmp(argv[1], "-1") != 0)
    {
        fgets(str, 56, stdin);
    }
    else
    {   
        strcpy(str, argv[0]);
    }

    // Remove \n
    int pos = (int)strcspn(str, "\n");
    if(pos < strlen(str)) str[pos] = '\0';

    reverse_string(str);

    printf("%s", str);
    if(strcmp(argv[2], "n") == 0) { printf("\n"); }
    
    return 1;
}
