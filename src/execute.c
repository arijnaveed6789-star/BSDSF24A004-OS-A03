#include "shell.h"

void setup_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {

        // Input redirection <
        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Input file missing\n");
                exit(1);
            }

            int fd = open(args[i + 1], O_RDONLY);

            if (fd < 0) {
                perror("Input file");
                exit(1);
            }

            dup2(fd, STDIN_FILENO);
            close(fd);

            for (int j = i; args[j + 1] != NULL; j++) {
                args[j] = args[j + 2];
            }

            args[i] = NULL;
            i--;
        }

        // Output redirection >
        else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Output file missing\n");
                exit(1);
            }

            int fd = open(args[i + 1],
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);

            if (fd < 0) {
                perror("Output file");
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);

            for (int j = i; args[j + 1] != NULL; j++) {
                args[j] = args[j + 2];
            }

            args[i] = NULL;
            i--;
        }
    }
}

int execute(char *arglist[]) {

    // Check whether command contains a pipe
    int pipe_index = -1;

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }

    // PIPE IMPLEMENTATION
    if (pipe_index != -1) {

        if (pipe_index == 0 || arglist[pipe_index + 1] == NULL) {
            fprintf(stderr, "Invalid pipe command\n");
            return -1;
        }

        int fd[2];

        if (pipe(fd) == -1) {
            perror("pipe");
            return -1;
        }

        arglist[pipe_index] = NULL;

        pid_t pid1 = fork();

        if (pid1 == 0) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);

            setup_redirection(arglist);
            execvp(arglist[0], arglist);
            perror("Command execution failed");
            exit(1);
        }

        pid_t pid2 = fork();

        if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);

            setup_redirection(&arglist[pipe_index + 1]);
            execvp(arglist[pipe_index + 1],
                   &arglist[pipe_index + 1]);

            perror("Command execution failed");
            exit(1);
        }

        close(fd[0]);
        close(fd[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);

        return 0;
    }

    // NORMAL COMMAND WITH REDIRECTION
    pid_t cpid = fork();

    if (cpid == -1) {
        perror("fork failed");
        return -1;
    }

    if (cpid == 0) {
        setup_redirection(arglist);

        execvp(arglist[0], arglist);
        perror("Command not found");
        exit(1);
    }

    waitpid(cpid, NULL, 0);
    return 0;
}
