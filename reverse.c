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

int main(int argc, char *argv[]) {
    char str[56];
    if(argv[1] == NULL)
    {
        fgets(str, 56, stdin);
    }
    else
    {   
        strcpy(str, argv[1]);
    }

    reverse_string(str);
    printf("%s\n", str);
    
    return 0;
}
