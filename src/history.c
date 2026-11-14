#include "shell.h"

#define HISTORY_SIZE 20

typedef struct {
    int number;
    char command[MAX_LEN];
} HistoryEntry;

static HistoryEntry history[HISTORY_SIZE];
static int history_count = 0;
static int next_number = 1;

void history_add(const char *cmd) {
    if (cmd == NULL || cmd[0] == '\0') {
        return;
    }

    // If history is full, remove the oldest command
    if (history_count == HISTORY_SIZE) {
        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history_count--;
    }

    history[history_count].number = next_number++;

    strncpy(history[history_count].command, cmd, MAX_LEN - 1);
    history[history_count].command[MAX_LEN - 1] = '\0';

    history_count++;
}

void history_print(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n",
               history[i].number,
               history[i].command);
    }
}

int expand_history_command(char *cmdline, char *expanded, int size) {
    if (cmdline[0] != '!' || cmdline[1] == '\0') {
        return 0;
    }

    char *end;
    long number = strtol(cmdline + 1, &end, 10);

    if (*end != '\0' || number <= 0) {
        printf("Invalid history command.\n");
        return -1;
    }

    for (int i = 0; i < history_count; i++) {
        if (history[i].number == number) {
            snprintf(expanded, size, "%s",
                     history[i].command);

            printf("%s\n", expanded);
            return 1;
        }
    }

    printf("History entry not found: %ld\n", number);
    return -1;
}
