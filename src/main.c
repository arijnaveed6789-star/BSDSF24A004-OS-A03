#include <ctype.h>
#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_IF_LINES 50

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

        if (strncmp(command, text, length) == 0)
            return strdup(command);
    }

    return NULL;
}

static char **command_completion(const char *text, int start, int end) {
    (void)end;

    if (start == 0)
        return rl_completion_matches(text, command_generator);

    return NULL;
}

static char *trim(char *str) {
    while (*str == ' ' || *str == '\t')
        str++;

    int len = strlen(str);

    while (len > 0 &&
          (str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[--len] = '\0';
    }

    return str;
}

static int is_builtin_command(const char *name) {
    return strcmp(name, "cd") == 0 ||
           strcmp(name, "exit") == 0 ||
           strcmp(name, "history") == 0 ||
           strcmp(name, "help") == 0 ||
           strcmp(name, "set") == 0 || strcmp(name, "jobs") == 0;
}

static int process_assignment(const char *line) {
    const char *equal = strchr(line, '=');

    if (equal == NULL || equal == line)
        return 0;

    int name_len = equal - line;

    if (!(isalpha((unsigned char)line[0]) || line[0] == '_'))
        return 0;

    for (int i = 1; i < name_len; i++) {
        if (!isalnum((unsigned char)line[i]) && line[i] != '_')
            return 0;
    }

    char *name = malloc(name_len + 1);
    if (name == NULL)
        return 1;

    strncpy(name, line, name_len);
    name[name_len] = '\0';

    const char *value = equal + 1;
    size_t value_len = strlen(value);

    char *stored_value = malloc(value_len + 1);

    if (stored_value == NULL) {
        free(name);
        return 1;
    }

    strcpy(stored_value, value);

    if (value_len >= 2 &&
        ((stored_value[0] == '"' &&
          stored_value[value_len - 1] == '"') ||
         (stored_value[0] == '\'' &&
          stored_value[value_len - 1] == '\''))) {

        memmove(stored_value, stored_value + 1, value_len - 2);
        stored_value[value_len - 2] = '\0';
    }

    if (!variable_set(name, stored_value))
        fprintf(stderr, "Unable to store variable.\n");

    free(name);
    free(stored_value);

    return 1;
}

static void expand_arguments(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '$' && args[i][1] != '\0') {
            const char *value = variable_get(args[i] + 1);

            char *replacement = strdup(value != NULL ? value : "");

            if (replacement != NULL) {
                free(args[i]);
                args[i] = replacement;
            }
        }
    }
}

static int run_command_line(char *line) {
    char *saveptr;
    char *segment = strtok_r(line, ";", &saveptr);
    int status = 0;

    while (segment != NULL) {
        segment = trim(segment);

        int len = strlen(segment);
        int background = 0;

        if (len == 0) {
            segment = strtok_r(NULL, ";", &saveptr);
            continue;
        }

                if (segment[len - 1] == '&') {
            background = 1;
            segment[--len] = '\0';
            segment = trim(segment);
        }

        if (process_assignment(segment)) {
            segment = strtok_r(NULL, ";", &saveptr);
            continue;
        }

        char **args = tokenize(segment);

        if (args != NULL) 
        {             expand_arguments(args);
            if (background && is_builtin_command(args[0])) {
                fprintf(stderr,
                        "Built-in commands cannot run in background.\n");
                status = 1;
            }
            else if (handle_builtin(args)) {
                status = 0;
            }
            else {
                status = execute(args, background, segment);
            }

            for (int i = 0; args[i] != NULL; i++)
                free(args[i]);

            free(args);
        }

        reap_jobs();
        segment = strtok_r(NULL, ";", &saveptr);
    }

    return status;
}

static void free_lines(char *lines[], int count) {
    for (int i = 0; i < count; i++)
        free(lines[i]);
}

static void execute_if_block(void) {
    char *condition = NULL;
    char *then_lines[MAX_IF_LINES];
    char *else_lines[MAX_IF_LINES];

    int then_count = 0;
    int else_count = 0;
    int in_else = 0;
    int finished = 0;

    char *input = readline("IF condition> ");

    if (input == NULL)
        return;

    input = trim(input);

    if (strlen(input) == 0) {
        free(input);
        printf("Condition missing.\n");
        return;
    }

    condition = strdup(input);
    free(input);

    input = readline("IF> ");

    if (input == NULL) {
        free(condition);
        return;
    }

    if (strcmp(trim(input), "then") != 0) {
        printf("Expected 'then'.\n");
        free(input);
        free(condition);
        return;
    }

    free(input);

    while (!finished) {
        input = readline("IF> ");

        if (input == NULL)
            break;

        char *line = trim(input);

        if (strcmp(line, "else") == 0) {
            if (in_else) {
                printf("Unexpected else.\n");
                free(input);
                break;
            }

            in_else = 1;
        }
        else if (strcmp(line, "fi") == 0) {
            finished = 1;
        }
        else if (strlen(line) > 0) {
            if (!in_else && then_count < MAX_IF_LINES) {
                then_lines[then_count++] = strdup(line);
            }
            else if (in_else && else_count < MAX_IF_LINES) {
                else_lines[else_count++] = strdup(line);
            }
            else {
                printf("Maximum block commands reached.\n");
            }
        }

        free(input);
    }

    if (!finished) {
        printf("Missing 'fi'.\n");
        free(condition);
        free_lines(then_lines, then_count);
        free_lines(else_lines, else_count);
        return;
    }

    int status = run_command_line(strdup(condition));

    if (status == 0) {
        for (int i = 0; i < then_count; i++)
            run_command_line(strdup(then_lines[i]));
    }
    else {
        for (int i = 0; i < else_count; i++)
            run_command_line(strdup(else_lines[i]));
    }

    free(condition);
    free_lines(then_lines, then_count);
    free_lines(else_lines, else_count);
}

int main(void) {
    char *cmdline;
    char expanded[MAX_LEN];

    rl_attempted_completion_function = command_completion;

    while (1) {
        reap_jobs();

        cmdline = readline(PROMPT);

        if (cmdline == NULL)
            break;

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

        char *input = trim(cmdline);

        if (strcmp(input, "if") == 0) {
            execute_if_block();
        }
        else {
            run_command_line(strdup(input));
        }

        free(cmdline);
        reap_jobs();
    }

    reap_jobs();
    printf("\nShell exited.\n");

    return 0;
}
