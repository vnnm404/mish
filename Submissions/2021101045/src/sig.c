#include "../include/global.h"

int sh_sig(int nargs, char *args[]) {
    if (nargs != 3) {
        printf("Here1\n");
        fprintf(stderr, "sig: invalid arguments\n");
        return TSK_FAIL;
    }

    char *jarg = args[1];
    char *sarg = args[2];

    for (int i = 0; i < strlen(jarg); i++) {
        if (!isdigit(jarg[i])) {
            printf("Here2\n");
            fprintf(stderr, "sig: invalid arguments\n");
            return TSK_FAIL;
        }
    }

    for (int i = 0; i < strlen(sarg); i++) {
        if (!isdigit(sarg[i])) {
            printf("Here3\n");
            fprintf(stderr, "sig: invalid arguments\n");
            return TSK_FAIL;
        }
    }

    int jid = atoi(jarg);
    int sig = atoi(sarg);

    struct sh_proc *h = sh_procs->head->next;
    while (h != NULL) {
        if (h->jid = jid) {
            if (kill(h->id, sig) == -1) {
                perror("sig");
                return TSK_FAIL;
            }

            break;
        }
        h = h->next;
    }

    return TSK_OK;
}