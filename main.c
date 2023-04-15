#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>

#define MAX_LEN 64
#define MAX_FILE_NAME_LEN 32
#define MAX_PATH_LEN 56

// predefined functions
char** parse(char* input);
void print_matrix(char** matrix, int len);
void ls(char** args, int position, char* currentPath);



void run_shell() 
{
    char input[MAX_LEN];
    int status;
    char** args;
    char currentPath[MAX_PATH_LEN];


    while (true)
    {
        printf("my-prompt $ ");
        fgets(input, MAX_LEN, stdin);
        args = parse(input);

        // Set current directory
        if(getcwd(currentPath, sizeof(currentPath)) == NULL) perror("error: Cannot access to current dir\n"); 


        if(strcmp(args[0], "pwd") == 0){
            printf("%s\n", currentPath);
        }
        else
        {
            __pid_t pid = fork(); // crear un proceso hijo
            if (pid == 0) 
            { // si el proceso es el hijo
                if (strcmp(args[0], "ls") == 0) { // comprobar si el usuario ingresó "ls"
                    execvp("./bin/ls", args); // ejecutar el comando ls con los argumentos proporcionados
                    printf("Error: Comando no encontrado.\n"); // si el comando falla, imprime un mensaje de error
                    exit(1);
                } else {
                    execvp(args[0], args); // ejecutar el comando con los argumentos proporcionados
                    printf("Error: Comando no encontrado.\n"); // si el comando falla, imprime un mensaje de error
                    exit(1);
                }
            } 
            else 
            { // si el proceso es el padre
                wait(NULL); // esperar a que el hijo termine
            }
        }
        

        
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

    char response[required_space][MAX_PATH_LEN];
    int i_save = 0;
    int len = strlen(input);
    char acum[MAX_LEN] = "";
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
                memset(acum, 0, MAX_LEN); // clean string
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
        parsed_response[i] = malloc(MAX_PATH_LEN * sizeof(char));
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