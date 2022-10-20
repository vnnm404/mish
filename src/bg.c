#include "../include/global.h"

int sh_bg(int nargs, char *args[]) {
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

    // printf("Here\n");
    h->stopped = 0;

    return TSK_OK;
}