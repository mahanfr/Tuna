#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LEN 1024

typedef struct {
    char* program_name;
    int redirect[2];
    char* args[];
} Cmd;

typedef struct {
    size_t n_cmds;
    Cmd* cmds[];
} Pipeline;

ssize_t prompt_and_get_input(const char* prompt,char **line, size_t *len) {
    fputs(prompt, stdout);
    return getline(line, len, stdin);
}

void print_command(Cmd* command) {
  char** arg = command->args;
  int i = 0;

  fprintf(stderr, "progname: %s\n", command->program_name);

  for (i = 0, arg = command->args; *arg; ++arg, ++i) {
    fprintf(stderr, " args[%d]: %s\n", i, *arg);
  }
}


char* next_non_empty(char **line) {
  char *tok;
  while ((tok = strsep(line, " \t\n\r")) && !*tok);
  return tok;
}

Cmd* parse_command(char *cmd_str) {
    char* copy = strndup(cmd_str, MAX_LEN);
    char* token;
    int i = 0;

    Cmd* cmd = calloc(sizeof(Cmd) + MAX_LEN * sizeof(char*), 1);
    while ((token = next_non_empty(&copy))) {
        cmd->args[i++] = token;
    }
    cmd->program_name = cmd->args[0];
    cmd->redirect[0] = cmd->redirect[1] = -1;

    free(copy);
    return cmd;
}

Pipeline* parse_pipeline(char *line) {
    char* copy = strndup(line, MAX_LEN);
    char* cmd_str;
    int n_cmds = 0;
    int i = 0;
    Pipeline* pipe;
    
    for(char* cur = copy; *cur; cur++) {
        if (*cur == '|') ++n_cmds;
    }
    ++n_cmds;

    pipe = calloc(sizeof(Pipeline) + n_cmds * sizeof(Cmd*), 1);
    pipe->n_cmds = n_cmds;

    while((cmd_str = strsep(&copy, "|"))) {
        pipe->cmds[i++] = parse_command(cmd_str);
    }
    free(copy);
    return pipe;
}

void close_all_pipes(int n_pips, int (*pipes)[2]) {
    for (int i = 0; i< n_pips; ++i) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

int exec_with_redir(Cmd* command, int n_pipes, int (*pipes)[2]) {
    int fd = -1;
    if ((fd = command->redirect[0] != -1)) {
        dup2(fd, STDIN_FILENO);
    }
    if ((fd = command->redirect[1] != -1)) {
        dup2(fd, STDOUT_FILENO);
    }
    close_all_pipes(n_pipes,pipes);
    return execvp(command->program_name, command->args);
}

pid_t run_with_redir(Cmd* command, int n_pipes, int (*pipes)[2]) {
    pid_t child_pid = fork();
    
    if (child_pid) {
        switch (child_pid) {
            case -1:
                fprintf(stderr, "Error: while creating a sub process\n");
                return -1;
            default:
                return child_pid;
        }
    } else {
        exec_with_redir(command,n_pipes,pipes);
        perror("Failed Process");
        return 0;
    }
}

int main(void) {
    char *line = NULL;
    size_t len = 0;

    while (prompt_and_get_input("[Tuna]$ ",&line, &len) != 0) {
        Pipeline *pipeline = parse_pipeline(line);
        int n_pipes = pipeline->n_cmds - 1;

        int (*pipes)[2] = calloc(sizeof(int[2]), n_pipes);

        for (size_t i=1; i< pipeline->n_cmds; ++i) {
            pipe(pipes[i-1]);
            pipeline->cmds[i]->redirect[STDIN_FILENO] = pipes[i-1][0];
            pipeline->cmds[i-1]->redirect[STDOUT_FILENO] = pipes[i-1][1];
        }

        for (size_t i = 0; i < pipeline->n_cmds; ++i) {
            run_with_redir(pipeline->cmds[i],n_pipes,pipes);
        }
        close_all_pipes(n_pipes, pipes);

        for (size_t i = 0;i < pipeline->n_cmds; ++i) {
            wait(NULL);
        }
    }
    fputs("\n", stdout);
    return 0;
}
