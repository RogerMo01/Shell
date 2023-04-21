#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char* historyDir = argv[0];

    FILE* file = fopen(historyDir, "r");
    char line[100];

    int i = 1;
    while (fgets(line, 100, file) != NULL)
    {
        printf("%i: %s", i, line);
        i++;
    }
    fclose(file);
}