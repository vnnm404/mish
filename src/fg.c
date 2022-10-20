#include "../include/global.h"
#include "../include/utils.h"

int sh_fg(int nargs, char *args[]) {
    if (nargs != 2) {
        fprintf(stderr, "fg: invalid arguments\n");
        return TSK_FAIL;
    }

    for (int i = 0; i < strlen(args[1]); i++) {
        if (!isdigit(args[1][i])) {
            fprintf(stderr, "fg: invalid arguments\n");
            return TSK_FAIL;
        }
    }

    int jid = atoi(args[1]);

    struct sh_proc *h = sh_procs->head->next;
    while (h != NULL) {
        if (h->jid == jid) {
            break;
        }
        h = h->next;
    }

    if (h == NULL) {
        fprintf(stderr, "fg: job not found\n");
        return TSK_FAIL;
    }

    if (kill(h->id, SIGCONT) == -1) {
        perror("fg: kill");
        return TSK_FAIL;
    }

    long start = micro_timestamp();

    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    tcsetpgrp(STDIN_FILENO, getpgid(h->id));

    int wstat;
    // struct rusage fg_rusage;
    // if (wait4(pchld, &wstat, WUNTRACED, &fg_rusage) == -1) {
    if (waitpid(h->id, &wstat, WUNTRACED) == -1) {
        perror("sh: waitpid");
        exit_gracefully(1);
    }

    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
        perror("ctty");
        exit_gracefully(1);
    }

    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    // printf("Took %ld\n",  micro_timestamp() - start);
    last_time_taken = micro_timestamp() - start;


    if (WIFEXITED(wstat)) {
        last_exit_stat = WEXITSTATUS(wstat);
        if (sh_proc_list_rem(sh_procs, h->id) == 2) {
            fprintf(stderr, "sh: proc_list: failed to find child (wtf)\n");
            exit_gracefully(1);
        }
    } else if (WIFSIGNALED(wstat)) {
        last_exit_stat = 1;
        if (sh_proc_list_rem(sh_procs, h->id) == 2) {
            fprintf(stderr, "sh: proc_list: failed to find child (wtf)\n");
            exit_gracefully(1);
        }
    } else if (WIFSTOPPED(wstat)) {
        /* handle ? */
        printf("Stopped\n");
    }

    return TSK_OK;
}