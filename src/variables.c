#include "shell.h"

typedef struct Variable {
    char *name;
    char *value;
    struct Variable *next;
} Variable;

static Variable *head = NULL;

int variable_set(const char *name, const char *value) {
    Variable *current = head;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            char *new_value = strdup(value);

            if (new_value == NULL)
                return 0;

            free(current->value);
            current->value = new_value;
            return 1;
        }

        current = current->next;
    }

    Variable *new_node = malloc(sizeof(Variable));

    if (new_node == NULL)
        return 0;

    new_node->name = strdup(name);
    new_node->value = strdup(value);

    if (new_node->name == NULL || new_node->value == NULL) {
        free(new_node->name);
        free(new_node->value);
        free(new_node);
        return 0;
    }

    new_node->next = head;
    head = new_node;

    return 1;
}

const char *variable_get(const char *name) {
    Variable *current = head;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0)
            return current->value;

        current = current->next;
    }

    return NULL;
}

void variables_print(void) {
    Variable *current = head;

    while (current != NULL) {
        printf("%s=%s\n", current->name, current->value);
        current = current->next;
    }
}
