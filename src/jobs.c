#include "shell.h"

#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
    int active;
} Job;

static Job job_list[MAX_JOBS];

void add_job(pid_t pid, const char *command) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!job_list[i].active) {
            job_list[i].pid = pid;
            snprintf(job_list[i].command, MAX_LEN, "%s", command);
            job_list[i].active = 1;

            printf("[%d] %d\n", i + 1, pid);
            return;
        }
    }

    fprintf(stderr, "Maximum job limit reached.\n");
}

void reap_jobs(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (job_list[i].active && job_list[i].pid == pid) {
                printf("\n[%d] Done  %s\n",
                       i + 1, job_list[i].command);
                job_list[i].active = 0;
                break;
            }
        }
    }
}

void print_jobs(void) {
    reap_jobs();

    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].active) {
            printf("[%d] Running  %d  %s\n",
                   i + 1,
                   job_list[i].pid,
                   job_list[i].command);
        }
    }
}
