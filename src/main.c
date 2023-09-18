#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

int pipe_counts(char* line) {
    int i = 0;
    int ret = 0;
    while (line[i] != '\0') {
        if (line[i] == '|') {
            ret++;
        }
        i++;
    }
    return ret;
}

Cmd* parse_command(char *line,int start, int end) {
    Cmd *cmd = calloc(sizeof(Cmd) + MAX_LEN * sizeof(char*), 1);
    return cmd;
}

Pipeline* parse_pipeline(char *line) {
    int i = 0;
    int start = 0;
    int end = 0; 
    int n_cmds = pipe_counts(line);
    int j = n_cmds;

    Pipeline* ret = calloc(sizeof(Pipeline) + n_cmds * sizeof(Cmd*), 1);
    while (line[i] != '\0') {
        if (line[i] == '|') {
            end = i - 1;
            ret->cmds[j] = parse_command(line, start, end); 
            start = i + 1;
            j--;
        }
        i++;
    }
    return ret;
}

int main(void) {
    char *line = NULL;
    size_t len = 0;

    while (prompt_and_get_input("[Shark]$ ",&line, &len) != 0) {
        Pipeline *pipe = parse_pipeline(line);
        printf("%s",line);
    }
    return 0;
}
