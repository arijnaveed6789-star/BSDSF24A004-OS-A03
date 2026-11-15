#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "

// Function prototypes
void add_job(pid_t pid, const char *command);
void reap_jobs(void);
void print_jobs(void);
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist, int background, const char *command);
int handle_builtin(char** arglist);
int expand_history_command(char *cmdline, char *expanded, int size);
void history_add(const char *cmd);
void history_print(void);
#endif // SHELL_H
