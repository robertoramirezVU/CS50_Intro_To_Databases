#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handle_cmd(node_t *node);
void handle_sequence(node_t *node);
void handle_pipe(node_t *node);
void signalHandler();

void initialize(void)
{
    if (prompt)
        prompt = "vush$ ";
}

void run_command(node_t *node)
{
    //print_tree(node);
    
    switch (node->type)
    {
        case NODE_COMMAND:
            handle_cmd(node);
            break;
        case NODE_PIPE:
            handle_pipe(node);
            break;
        case NODE_SEQUENCE:
            handle_sequence(node);
            break;
        case NODE_REDIRECT:
            break;
        case NODE_SUBSHELL:
            break;
        case NODE_DETACH:
            break;
        default:
            break;
    }

    if (prompt)
        prompt = "vush$ ";
}
void handle_pipe(node_t *node) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(-1);
    } else {

        int pid = fork();
        if (pid == 0) {

            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);

            close(pipefd[1]);
            handle_cmd(node->pipe.parts[0]);

            exit(0);
        } else if(pid == -1){
            perror("fork");
            exit(-1);
        } else {

            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            handle_cmd(node->pipe.parts[1]);

            int status;
            waitpid(pid, &status, 0);
        }
    }
}

void signalHandler(){
  signal(SIGINT,SIG_DFL);
}


void handle_sequence(node_t *node){
    if (node->sequence.first != NULL) {
        handle_cmd(node->sequence.first);
    }
    if (node->sequence.second != NULL) {
        run_command(node->sequence.second);
    }
}


void handle_cmd(node_t *node) {
    if (strcmp(node->command.program, "cd") == 0) 
    {
            chdir(node->command.argv[1]);
    } 
    else if (strcmp(node->command.program, "exit") == 0) 
    {
         exit(atoi(node->command.argv[1]));
    } 
    else 
    {
        int status;
        int pid = fork();
        if (pid == 0) 
        {
            signal(SIGINT, SIG_DFL);
            execvp(node->command.program, node->command.argv);
            perror("Error: child process failed");
             exit(atoi(node->command.argv[1]));
        } 
        else 
        {
            signal(SIGINT, signalHandler);
            if (strcmp(node->command.program, "exit") == 0) 
            {
                waitpid(pid, &status, 0);
            } else 
            {
                waitpid(pid, &status, 0);
            }
        }
    }
}

