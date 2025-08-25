#include<stdio.h>    // for printf(), perror()
#include<stdlib.h>  // for malloc(), realloc(), free(), exit(), EXIT_SUCCESS, EXIT_FAILURE
#include<string.h>    // for strtok(), strcmp()
#include <unistd.h>    // for fork(), execvp(), chdir(), pid_t
#include <sys/wait.h>  // for waitpid(), WUNTRACED

#define delimiters " \t\r\n"
#define buffer_size 128

// Function declaractions

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

// Global var
char *builtins[] = {"cd","help","exit"};

int (*builtin_funcs[]) (char**) = { &shell_cd, &shell_help, &shell_exit};

int num_builtins = sizeof(builtins) / sizeof(char*);

// Function implementations

int shell_cd(char ** args){
    if(args[1]==NULL)
        perror("Expected a directory");
    else{
        if(chdir(args[1])!=0)
            perror("chdir failed");
        else
            printf("Directory changed to %s\n", args[1]);
    }
    return 1;
}


int shell_help(char ** args){
    printf("Shell Help:\n");
    printf("The following are available functions:\n");
    for(int i=0;i<num_builtins;i++){
        printf("%s\n", builtins[i]);
    }
    return 1;
}

int shell_exit(char ** args){
    return 0;
}



char * shell_read_line(){
    char *line = NULL;
    size_t bufsize = 0; 
    // getline will allocate buffer as needed
    if(getline(&line, &bufsize, stdin)==-1){
        if(feof(stdin)) // to check whether we reached EOF
            exit(EXIT_SUCCESS);
        else{
            perror("reading line");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char** shell_parse_line(char *line){
    int buff = buffer_size; 
    char **tokens = malloc(buff * sizeof(char*));
    char *token;
    int position = 0;

    if(!tokens){
        perror("Cannot allocate memory for tokens");
        exit(EXIT_FAILURE); 
    }
    token = strtok(line, delimiters);
    while(token!=NULL){
        tokens[position] = token;
        position++;

        if(position>=buff){
            buff += buffer_size;
            tokens = realloc(tokens, buff * sizeof(char*));
            if(!tokens){
                perror("Cannot reallocate memory for tokens");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, delimiters);
    }
    tokens[position] = NULL; // Null-terminate the array of tokens
    return tokens;
}


int shell_launch(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if(pid==0){     //Child process
        if(execvp(args[0], args)==-1){
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }
    else if(pid<0)
        perror("fork failed");
    else{    // Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}


int shell_execute(char **args){
    if(args[0]==NULL)
        return 1;
    for(int i=0;i<num_builtins;i++){
        if(strcmp(args[0], builtins[i])==0){
            return (*builtin_funcs[i])(args);
        }
    }
    return shell_launch(args);
}


void shell_loop(){
    char *line;
    char **args;
    int status;
    // For basic operations which are read, understand and execute
    do{
        printf("shell>");   
        line = shell_read_line();
        args = shell_parse_line(line);
        status = shell_execute(args);
        free(line);
        free(args);
    }while (status);

}



int main(int argc, char **argv){
    shell_loop();
    return EXIT_SUCCESS;
}