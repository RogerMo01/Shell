#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LEN 64
// predefined functions
char** parse(char* input);
void print_matrix(char** matrix, int len);



void run_shell() 
{
    char input[MAX_LEN];
    int status;
    char** args;

    while (true)
    {
        printf("my-prompt $ ");
        fgets(input, MAX_LEN, stdin);
        args = parse(input);



    }
}

char** parse(char* input)
{
    bool last_space = false;
    int count = 0;
    
    // count words
    for(int i=0; i<strlen(input); i++)
    {
        if(i==0){last_space = true; continue;}
        if(i == (int)(strlen(input)-1) && last_space) count--;

        if(input[i] == ' '){
            if (!last_space){
                count++;
            }
            last_space = true;
        }
        else last_space = false;
    }
    int count_words = count+1;
    int required_space = count_words+1;

    char response[required_space][56];
    int i_save = 0;
    int len = strlen(input);
    char acum[64] = "";
    last_space = true;

    for(int i=0; i < len -1; i++)
    {
        // printf("i = %i\n", i);
        // printf("input = %c\n", input[i]);
        
        if(input[i] == ' ')
        {
            if(!last_space)
            {
                strcpy(response[i_save], acum);
                memset(acum, 0, 64); // clean string
                i_save+=1;
            }
            last_space=true;
        }
        else{
            char tmp[2] = "";
            tmp[0] = input[i];
            char* t = tmp;
            strcat(acum, t);
            last_space = false;

            if(i == len-2)
            {
                strcpy(response[i_save], acum);
            }
        }
    }

    char** parsed_response = malloc(count_words * sizeof(char*));
    for (int i = 0; i < count_words; i++)
    {
        parsed_response[i] = malloc(56 * sizeof(char));
        strcpy(parsed_response[i], response[i]);
    }
    parsed_response[count_words] = NULL;

    // print_matrix(parsed_response, required_space);
    return parsed_response;
}



void print_matrix(char** matrix, int len)
{
    printf("~~~ Matrix: ~~~\n");
    for (int i = 0; i < len; i++)
    {
        if(matrix[i] == NULL)
        {
            printf("NULL\n");
        }
        else
        {
            printf("%s\n", matrix[i]);
        }
    }
    printf("~~~~ End ~~~~\n");
}

int main(void) {
    run_shell();
    return 0;
}