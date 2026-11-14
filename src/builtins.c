#include "shell.h"

int handle_builtin(char** arglist) {

    if (strcmp(arglist[0], "exit") == 0) {
        exit(0);
    }

    else if (strcmp(arglist[0], "cd") == 0) {

        if (arglist[1] == NULL) {
            char* home = getenv("HOME");

            if (home != NULL && chdir(home) != 0) {
                perror("cd");
            }
        }
        else if (chdir(arglist[1]) != 0) {
            perror("cd");
        }

        return 1;
    }

    else if (strcmp(arglist[0], "help") == 0) {

        printf("Available commands:\n");
        printf("exit - Exit the shell\n");
        printf("cd - Change directory\n");
        printf("help - Display help information\n");
        printf("jobs - Display background jobs\n");

        return 1;
    }

    else if (strcmp(arglist[0], "jobs") == 0) {

        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0;
}
