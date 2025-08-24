#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define delimiters " "
#define buffer_size 128

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


int shell_execute(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if(pid==0){     //Child process
        execvp(args[0], args);
    }
    else{    // Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 0;
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
        // free(line);
        // free(args);
    }while (status);

}



int main(int argc, char **argv){
    shell_loop();
    return EXIT_SUCCESS;
}