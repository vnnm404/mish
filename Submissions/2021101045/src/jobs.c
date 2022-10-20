#include "../include/global.h"

int cmp_name(const void *a, const void *b) {
    int aid = *(int *) a;
    int bid = *(int *) b;

    char *aname, *bname;
    for (struct sh_proc *h = sh_procs->head->next; h != NULL; h = h->next) {
        if (h->id == aid)
            aname = h->name;
        if (h->id == bid)
            bname = h->name;
    }

    return strcmp(aname, bname);
}

int sh_jobs(int nargs, char *args[]) {
    char r_flag = 1;
    char s_flag = 1;

    for (int i = 1; i < nargs; i++) {
        char *arg = args[i];
        if (*arg != '-') {
            fprintf(stderr, "jobs: invalid arguments\n");
            return TSK_FAIL;
        }

        for (arg = arg + 1; *arg != '\0'; arg++) {
            if (*arg == 'r')
                s_flag = 0;
            if (*arg == 's')
                r_flag = 0;
        }
    }

    int nprocs = 0;
    int sorted[BG_MAX];
    struct sh_proc *p = sh_procs->head->next;
    while (p != NULL) {
        nprocs++;

        char proc_file[PATH_MAX];
        struct stat proc_stat;
        sprintf(proc_file, "/proc/%d", p->id);

        if (stat(proc_file, &proc_stat) == -1) {
            fprintf(stderr, "sh: pinfo: invalid pid\n");
            return TSK_FAIL;
        }

        char proc_file_status[PATH_MAX];
        strcpy(proc_file_status, proc_file);
        strcat(proc_file_status, "/stat");

        FILE *proc_id_status = fopen(proc_file_status, "r");
        if (proc_id_status == NULL) {
            fprintf(stderr, "sh: jobs: failed to open %s\n", proc_file_status);
            return TSK_FAIL;
        }

        pid_t store_id;
        char comm[PATH_MAX];
        char state;

        fscanf(proc_id_status, "%d %s %c", &store_id, comm, &state);

        if (state == 'T')
            p->stopped = 1;

        if (r_flag) {
            if (!p->stopped) {
                sorted[nprocs - 1] = p->id;
            }
        }
        
        if (s_flag) {
            if (p->stopped) {
                sorted[nprocs - 1] = p->id;
            }
        }

        p = p->next;
    }

    qsort(sorted, nprocs, sizeof(int), cmp_name);
    for (int i = 0; i < nprocs; i++) {
        struct sh_proc *p = sh_procs->head->next;
        while (p != NULL) {
            if (p->id == sorted[i])
                break;
            
            p = p->next;
        }

        if (p == NULL)
            continue;

        if (r_flag)
            if (!p->stopped)
                printf("[%d] Running %s [%d]\n", p->jid, p->name, p->id);
            

        if (s_flag)
            if (p->stopped)
                printf("[%d] Stopped %s [%d]\n", p->jid, p->name, p->id);
    }

    return TSK_OK;
}