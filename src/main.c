#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;
    char expanded[MAX_LEN];

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // Expand !n before adding the command to history
        int result = history_expand(cmdline, expanded, MAX_LEN);

        if (result == -1) {
            free(cmdline);
            continue;
        }

        if (result == 1) {
            snprintf(cmdline, MAX_LEN, "%s", expanded);
        }

        // Save command in history
        history_add(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {

            if (!handle_builtin(arglist)) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }

            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
