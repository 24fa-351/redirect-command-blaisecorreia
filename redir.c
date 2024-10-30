#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: redir <inp> <cmd> <out>\n");
        return 1;
    }

    char *inp = argv[1];
    char *cmd = argv[2];
    char *out = argv[3];

    char *args[100];
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    char *path = args[0];
    if (path[0] != '/') {
        path = getenv("PATH");
        if (path == NULL) {
            perror("getenv");
            return 1;
        }

        char cmd_path[1024];
        while (path != NULL) {
            char *p = strchr(path, ':');
            int len = p ? p - path : strlen(path);
            snprintf(cmd_path, len + strlen(args[0]) + 2, "%.*s/%s", len, path, args[0]);
            if (access(cmd_path, X_OK) == 0) {
                args[0] = cmd_path;
                break;
            }
            path = p ? p + 1 : NULL;
        }
        if (path == NULL) {
            fprintf(stderr, "Command not found\n");
            return 1;
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (strcmp(inp, "-") != 0) {
            int fd_in = open(inp, O_RDONLY);
            if (fd_in == -1) {
                perror("open input file");
                return 1;
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (strcmp(out, "-") != 0) {
            int fd_out = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("open output file");
                return 1;
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args);
        perror("execvp");
        return 1;
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
        return 1;
    }

    return 0;
}
