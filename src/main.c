#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    print_command(cmd);
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

int main(void) {
    char *line = NULL;
    size_t len = 0;

    while (prompt_and_get_input("[Shark]$ ",&line, &len) != 0) {
        Pipeline *pipe = parse_pipeline(line);
        int n_pipes = pipe->n_cmds - 1;
    }
    return 0;
}
