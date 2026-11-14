#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

static char *commands[] = {
    "cd", "exit", "help", "history", "jobs",
    "pwd", "ls", "cat", "mkdir", "rm",
    "clear", "echo", "whoami", "date", "touch",
    NULL
};

static char *command_generator(const char *text, int state) {
    static int index;
    static int length;

    if (state == 0) {
        index = 0;
        length = strlen(text);
    }

    while (commands[index] != NULL) {
        char *command = commands[index++];

        if (strncmp(command, text, length) == 0) {
            return strdup(command);
        }
    }

    return NULL;
}

static char **command_completion(const char *text, int start,
                                  int end) {
    (void)end;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    return NULL;
}

int main() {
    char *cmdline;
    char expanded[MAX_LEN];

    rl_attempted_completion_function = command_completion;

    while ((cmdline = readline(PROMPT)) != NULL) {

        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        int result = expand_history_command(cmdline, expanded, MAX_LEN);

        if (result == -1) {
            free(cmdline);
            continue;
        }

        if (result == 1) {
            free(cmdline);
            cmdline = strdup(expanded);
        }

        history_add(cmdline);
        add_history(cmdline);

        char **args = tokenize(cmdline);

        if (args != NULL) {
            if (!handle_builtin(args)) {
                execute(args);
            }

            for (int i = 0; args[i] != NULL; i++) {
                free(args[i]);
            }

            free(args);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
