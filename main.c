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
#define MAX_PATH_ARG 56
#define MAX_PATH_LEN 128
#define MAX_EXE_NAME 30

// Predefined functions
char** parse(char* input);
void print_matrix(char** matrix, int len);
void RedirectInput(char** args, int rdIn1_i);
void RedirectOutput(char** args, int rdOut1_i);
void ExecuteCommand(char** args, int cmd_i, char* runningDir, int rdIn);
void InitHistory();
void AddToHistory(char in[MAX_LEN]);
void GetAgainCMD(int i, char newInput[MAX_LEN]);
bool ValidateCommand(char* in, char** parsedIn);
bool AssertCmd(int start, int end, char** pIn, int rdIn, int rdOut, bool isPipe, bool isFirst);


// Running directory
char runningDir[MAX_PATH_LEN];
char historyDir[MAX_PATH_LEN];


void run_shell() 
{
    char input[MAX_LEN];
    int status;
    char** args;
    char currentPath[MAX_PATH_ARG];
    

    while (true)
    {
        printf("\x1B[33m" "our-prompt $ " "\x1B[0m");
        fgets(input, MAX_LEN, stdin);
        args = parse(input);

        if(args[0] == NULL) { fprintf(stderr, "Error: Command not found.\n"); exit(0); }

        // Command Validator
        if(!ValidateCommand(input, args)) continue;;

        // Catch again command
        if(strcmp(args[0], "again") == 0)
        {
            char newInput[sizeof(input)];
            int i = (int)strtol(args[1], NULL, 10);
            GetAgainCMD(i, newInput);

            input[0] = '\0';
            strcpy(input, newInput);
            args = parse(input);
        }

        // Save input
        AddToHistory(input);

        // In-Out redirectors positions
        int rdIn1_i = -1;
        int rdOut1_i = -1;

        // Pipe operators
        int pipe_i = -1;
        int rdOut2_i = -1;

        // Detect redirectors
        int i = 0;
        while (args[i] != NULL)
        {
            if(pipe_i == -1)
            {
                if(strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) rdOut1_i = i;
                if(strcmp(args[i], "<") == 0) rdIn1_i =  i;
            }
            else
            {
                if(strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) rdOut2_i = i;
            }
            
            if(strcmp(args[i], "|") == 0) pipe_i = i;
            i++;
        }


        // Set current directory
        if(getcwd(currentPath, sizeof(currentPath)) == NULL) fprintf(stderr, "Error: cannot access to current dir\n"); 

        if(strcmp(args[0], "exit") == 0)
        {
            printf("logout\n");
            exit(0);
        }
        else if(strcmp(args[0], "cd") == 0)
        {
            if(chdir(args[1]) != 0){
                fprintf(stderr, "Error: cannot change to this directory\n");
            }
        }
        else
        {
            __pid_t pid, pid1, pid2;
            int pipe_fd[2];
            pipe(pipe_fd);

            // NO pipe case
            if(pipe_i == -1)
            {
                // Create process
                pid = fork();
                if(pid == -1) fprintf(stderr, "Error: cannot fork the process\n");

                if (pid == 0) //child process
                { 
                    // Redirect Input Case
                    if(rdIn1_i > 0)
                    { RedirectInput(args, rdIn1_i); }

                    // Redirect Output Cases
                    if(rdOut1_i > 0)
                    { RedirectOutput(args, rdOut1_i); }

                    // Execute command
                    ExecuteCommand(args, 0, runningDir, rdIn1_i);
                } 
            }
            else
            {
                pid1 = fork();
                if(pid1 == 0)
                {
                    if(rdIn1_i > 0)
                    { RedirectInput(args, rdIn1_i); }

                    close(pipe_fd[0]);
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    
                    ExecuteCommand(args, 0, runningDir, rdIn1_i);
                    fprintf(stderr, "Error: child process\n");
                    exit(EXIT_FAILURE);
                }

                pid2 = fork();
                if(pid2 == 0)
                {
                    close(pipe_fd[1]);
                    dup2(pipe_fd[0], STDIN_FILENO);
                    
                    if(rdOut2_i > 0)
                    { RedirectOutput(args, rdOut2_i); }

                    ExecuteCommand(args, pipe_i + 1, runningDir, 1);
                    fprintf(stderr, "Error: child process 2\n");
                    exit(EXIT_FAILURE);
                }
            }

            close(pipe_fd[0]);
            close(pipe_fd[1]);
            
            waitpid(pid, NULL, 0);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
        
    }//while true
}


bool ValidateCommand(char* in, char** pIn)
{
    if(strlen(in) > MAX_LEN-2) { fprintf(stderr, "Error: the input len has exceeded supported len.\n"); exit(0); }

    int pipe = -1;
    int rdIn = -1;
    int rdOut = -1;
    int rdIn2 = -1;
    int rdOut2 = -1;

    int i = 0;
    while (pIn[i] != NULL)
    {
        if(strcmp(pIn[i], "|") == 0)
        {
            if(pipe != -1) { fprintf(stderr, "Error: no multipipe allowed.\n"); return false; }
            pipe = i;
        }
        if(pipe == -1)
        {
            if(strcmp(pIn[i], "<") == 0) rdIn =  i;
            if(strcmp(pIn[i], ">") == 0 || strcmp(pIn[i], ">>") == 0) rdOut = i;
        }
        else
        { 
            if(strcmp(pIn[i], "<") == 0) rdIn2 =  i;
            if(strcmp(pIn[i], ">") == 0 || strcmp(pIn[i], ">>") == 0) rdOut2 = i;
        }
        i++;
    }
    // printf("pipe = %i\n", pipe);
    // printf("rdIn = %i\n", rdIn);
    // printf("rdOut = %i\n", rdOut);
    // printf("rdOut2 = %i\n", rdOut2);

    if(pipe != -1)
    {
        if(!AssertCmd(0, pipe+1, pIn, rdIn, rdOut, true, true)) return false;
    }
    else
    {
        if(!AssertCmd(0, i, pIn, rdIn, rdOut, false, false)) return false;
    }

    return true;
}

bool AssertCmd(int start, int end, char** pIn, int rdIn, int rdOut, bool isPipe, bool isFirstPipe)
{
    // printf("assert cmd\n");
    // printf("pIn[start] = %s\n", pIn[start]);
    // printf("start = %i\n", start);
    // printf("end = %i\n", end);

    int i = start;

    if(isPipe)
    {
        if(isFirstPipe)
        {
            if(rdOut != -1) { fprintf(stderr, "Error: first pipe command cannot have '>' or '>>'.\n"); return false; }
        }
        else
        {
            if(rdIn != -1) { fprintf(stderr, "Error: second pipe command cannot have '<'.\n"); return false; }
        }
    }

    int len = end - start;
    
    if(strcmp(pIn[start], "again") == 0)
    {
        if(len == 1) { fprintf(stderr, "Error: you must provide a number argument in the range 1-10.\n"); return false; }
        int n = (int)strtol(pIn[start+1], NULL, 10);
        if(n > 10 || n < 1) { fprintf(stderr, "Error: argument must be a number in the range 1-10.\n"); return false; }
        if(len > 2 && strcmp(pIn[start+2], "<") == 0) { fprintf(stderr, "Error: cannot change stdin for again command.\n"); return false; }
    }
    else if(strcmp(pIn[start], "cd") == 0)
    {
        if(len == 1) { fprintf(stderr, "Error: you must provide a directory to change to.\n"); return false; }
        if(len > 2) { fprintf(stderr, "Error: too many arguments.\n"); return false; }
    }
    else if(strcmp(pIn[start], "exit") == 0)
    {
        
    }
    else if(strcmp(pIn[start], "pwd") == 0)
    {
        if(len > 1 && strcmp(pIn[start+1], "<") == 0) { fprintf(stderr, "Error: cannot change stdin for pwd command.\n"); return false; }
    }
    else if(strcmp(pIn[start], "ls") == 0)
    {
        if(len > 1 && (strcmp(pIn[start+1], ">") != 0 && strcmp(pIn[start+1], ">>") != 0)) { fprintf(stderr, "Error: this command takes no arguments.\n"); return false; }
        if(len > 1 && strcmp(pIn[start+1], "<") == 0) { fprintf(stderr, "Error: cannot change stdin for pwd command.\n"); return false; }
    }
    else if(strcmp(pIn[start], "echo") == 0)
    {
        
    }
    else if(strcmp(pIn[start], "reverse") == 0)
    {
        
    }
    else if(strcmp(pIn[start], "history") == 0)
    {
        if(len > 1 && (strcmp(pIn[start+1], ">") != 0 || strcmp(pIn[start+1], ">>") != 0)) { fprintf(stderr, "Error: this command take no arguments.\n"); return false; }
        if(len > 1 && strcmp(pIn[start+1], "<") == 0) { fprintf(stderr, "Error: cannot change stdin for history command.\n"); return false; }
    }
    else if(strcmp(pIn[start], "help") == 0)
    {   
        if(len == 3 && strcmp(pIn[start+1], ">") != 0 && strcmp(pIn[start+1], ">>") != 0) { fprintf(stderr, "Error: invalid syntax command\n"); return false; }
        if(len != 1 && len !=2 && (strcmp(pIn[start+1], "<") == 0 || strcmp(pIn[start+2], "<") == 0)) { fprintf(stderr, "Error: cannot change stdin for help command.\n"); return false; }
        if((len == 4 && strcmp(pIn[start+2], ">") != 0 && strcmp(pIn[start+2], ">>")) || len > 4) { fprintf(stderr, "Error: invalid syntax command\n"); return false; }
        if(len == 2 || len == 4)
        {
            char* topics[] = {"again", "cd", "echo", "exit", "history", "ls", "reverse", "pwd"};
            bool exist = false;
            for (int i = 0; i < 8; i++)
            {
                if(strcmp(topics[i], pIn[start+1]) == 0) { exist = true; break; }
            }
            if(!exist) { fprintf(stderr, "Error: no help topics match %s.\n", pIn[start+1]); return false; }
        }
    }
    else
    {
        fprintf(stderr, "Error: command not found.\n");
        return false;
    }
    
    return true;
}


void RedirectInput(char** args, int rdIn)
{
    int index = rdIn+1;
    int in_fd = open(args[index], O_RDONLY);
    if(in_fd == -1) { printf("Error: Cannot open input file\n"); exit(EXIT_FAILURE); }

    int dup2i = dup2(in_fd, 0);
    if(dup2i == -1) { printf("Error: Cannot change stdIn\n"); exit(EXIT_FAILURE); }

    close(in_fd);
}

void RedirectOutput(char** args, int rdOut)
{
    int index = rdOut+1;
    int out_fd = (strcmp(args[rdOut], ">>") == 0) ? open(args[index], O_WRONLY | O_CREAT | O_APPEND, 0644) : open(args[index], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    int dup2o = dup2(out_fd, 1);
    if(dup2o == -1) { printf("Error: Cannot change stdOut\n"); exit(EXIT_FAILURE); }

    close(out_fd);
}

void ExecuteCommand(char** args, int cmd_i, char* runningDir, int rdIn)
{
    // Set executable path
    char exeDir[sizeof(runningDir)+5+MAX_EXE_NAME]; // 5 characters of "/bin/" and 15 of executable name
    
    strcpy(exeDir, runningDir);

    char rdIn_arg[5];
    sprintf(rdIn_arg, "%d", rdIn);


    if(strcmp(args[cmd_i], "help") == 0)
    {
        strcat(exeDir, "/bin/help");

        char txtDir[sizeof(runningDir)+5+MAX_EXE_NAME];
        strcpy(txtDir, runningDir);
        strcat(txtDir, "/data/");

        char* help_arg = args[cmd_i+1];

        execl(exeDir, txtDir, help_arg, rdIn_arg, NULL);
    }

    strcat(exeDir, "/bin/");

    if(strcmp(args[cmd_i], "pwd") == 0)
    {
        strcat(exeDir, "pwd");
        execvp(exeDir, args);
    }
    else if(strcmp(args[cmd_i], "ls") == 0)
    {
        strcat(exeDir, "ls");
        execvp(exeDir, args);
    }
    else if(strcmp(args[cmd_i], "echo") == 0)
    {
        strcat(exeDir, "echo");
        execl(exeDir, args[cmd_i+1], rdIn_arg, NULL);
    }
    else if(strcmp(args[cmd_i], "reverse") == 0)
    {
        strcat(exeDir, "reverse");
        execl(exeDir, args[cmd_i+1], rdIn_arg, NULL);
    }
    else if(strcmp(args[cmd_i], "history") == 0)
    {
        strcat(exeDir, "history");
        execl(exeDir, historyDir, NULL);
    }
}

void GetAgainCMD(int i, char newInput[MAX_LEN])
{
    // Copy to newInput the new input that again command refers
    FILE* file = fopen(historyDir, "r");
    char line[100];
    int counter = 1;
    while (fgets(line, 100, file) != NULL)
    {
        if(counter == i)
        {
            strcpy(newInput, line);
            break;
        }
        counter++;
    }
    fclose(file);
}

void InitHistory()
{
    strcpy(historyDir, runningDir);
    strcat(historyDir, "/data/history.txt");
    int history_fd = open(historyDir, O_CREAT, 0644);
    close(history_fd);
}

void AddToHistory(char in[MAX_LEN])
{
    if(in[0] == ' ') return;

    FILE* file = fopen(historyDir, "r");

    char line[100];

    // Count lines
    int lineCounter = 0;
    while (fgets(line, 100, file) != NULL) lineCounter++;
    fclose(file);

    if(lineCounter == 10)
    {
        char tempDir[MAX_PATH_LEN];
        strcpy(tempDir, runningDir);
        strcat(tempDir, "/data/temp.txt");

        // copy history > temp
        FILE* temp = fopen(tempDir, "a");
        file = fopen(historyDir, "r");
        bool first = true;
        while (fgets(line, 100, file) != NULL)
        {
            if(first) {first = false; continue;}
            fprintf(temp, "%s", line);
        }
        fclose(temp);
        fclose(file);

        // reset history
        file = fopen(historyDir, "w");
        fclose(file);

        // copy temp > history
        temp = fopen(tempDir, "r");
        file = fopen(historyDir, "a");
        while (fgets(line, 100, temp) != NULL)
        {
            fprintf(file, "%s", line);
        }
        fclose(temp);
        fclose(file);

        remove(tempDir);
    }
    

    file = fopen(historyDir, "a");
    fprintf(file, "%s", in);
    fclose(file);
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
        if(input[i] == '#')
        {
            if(input[i-1] != ' ') count++;
            count--;
            break;
        }

        if(input[i] == ' ')
        {
            if (!last_space) count++;
            last_space = true;

            if(i>0 && input[i-1]=='\\' ) count--;
        }
        else last_space = false;

    }
    int count_words = count+1;
    int required_space = count_words+1;

    char response[required_space][MAX_PATH_ARG];
    int i_save = 0;
    int len = strlen(input);
    char acum[MAX_LEN] = "";
    last_space = true;
    int includeSpace = false;

    for(int i=0; i < len -1; i++)
    {
        if(input[i] == '\\') continue;
        if(i > 0 && input[i-1] == '\\') includeSpace = true;

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
        else
        {
            if(input[i] == '#')
            {
                if(!last_space) strcpy(response[i_save], acum);
                break;
            }

            char tmp[2] = "";
            tmp[0] = input[i];
            char* t = tmp;
            strcat(acum, t);
            last_space = false;
            includeSpace = false;

            if(i == len-2) strcpy(response[i_save], acum);
        }
    }

    char** parsed_response = malloc(count_words * sizeof(char*));
    for (int i = 0; i < count_words; i++)
    {
        parsed_response[i] = malloc(MAX_PATH_ARG * sizeof(char));
        strcpy(parsed_response[i], response[i]);
    }
    parsed_response[count_words] = NULL;

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

    getcwd(runningDir, sizeof(runningDir));
    InitHistory();

    run_shell();
    return 0;
}