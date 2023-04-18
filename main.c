#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LEN 64
#define MAX_FILE_NAME_LEN 32
#define MAX_PATH_LEN 56

// Predefined functions
char** parse(char* input);
void print_matrix(char** matrix, int len);


void run_shell() 
{
    char input[MAX_LEN];
    int status;
    char** args;
    char currentPath[MAX_PATH_LEN];


    while (true)
    {
        printf("\x1B[33m" "our-prompt $ " "\x1B[0m");
        fgets(input, MAX_LEN, stdin);
        args = parse(input);

        // In-Out redirectors positions
        int rdIn = -1;
        int rdOut = -1;

        // Detect redirectors
        int i = 0;
        while (args[i] != NULL)
        {
            if(strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) rdOut = i;
            if(strcmp(args[i], "<") == 0) rdIn =  i;
            i++;
        }

        // Set current directory
        if(getcwd(currentPath, sizeof(currentPath)) == NULL) perror("Error: Cannot access to current dir\n"); 


        if(strcmp(args[0], "cd") == 0)
        {
            if(args[1] == NULL){
                printf("Error: You must provide a directory to change to.\n");
            }
            else{
                if(chdir(args[1]) != 0){
                    perror("Error: Cannot change to this directory");
                }
            }
        }
        else
        {
            // Create process
            __pid_t pid = fork();
            if(pid == -1) perror("Error: Cannot fork the process\n");

            if (pid == 0) //child process
            { 
                // Redirect Input Case
                if(rdIn > 0)
                {
                    int in_fd = open(args[rdIn+1], O_RDONLY);
                    if(in_fd == -1) { perror("Error: Cannot open input file\n"); exit(EXIT_FAILURE); }

                    int dup2i = dup2(in_fd, 0);
                    if(dup2i == -1) { perror("Error: Cannot change stdIn\n"); exit(EXIT_FAILURE); }

                    close(in_fd);
                }

                // Redirect Output Cases
                if(rdOut > 0)
                {
                    int index = rdOut+1;
                    int out_fd = (strcmp(args[rdOut], ">>") == 0) ? open(args[index], O_WRONLY | O_CREAT | O_APPEND, 0644) : open(args[index], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    
                    int dup2o = dup2(out_fd, 1);
                    if(dup2o == -1) { perror("Error: Cannot change stdOut\n"); exit(EXIT_FAILURE); }

                    close(out_fd);
                }



                // Execute command
                if(strcmp(args[0], "pwd") == 0)
                {
                    execvp("./bin/pwd", args);
                }
                else if(strcmp(args[0], "ls") == 0)
                {
                    execvp("./bin/ls", args);
                }
                else
                {
                    printf("Error: Comando no encontrado.\n");
                    exit(1);
                }
                
            } 
            else //parent process
            {
                wait(NULL);
            }
        }

        
    }//while true
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

            if(i>0 && input[i-1]=='\\' ) count--;
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
    int includeSpace = false;

    for(int i=0; i < len -1; i++)
    {
        // printf("i = %i\n", i);
        // printf("input = %c\n", input[i]);
        
        if(input[i] == '\\') continue;
        if(i > 0 && input[i-1] == '\\')
        {
            includeSpace = true;
        }
        if(input[i] == ' ' && !includeSpace)
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
            includeSpace = false;

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