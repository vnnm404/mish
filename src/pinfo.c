#include "../include/global.h"
#include "../include/utils.h"
#include "../include/pinfo.h"

int sh_pinfo(int nargs, char *args[]) {
    pid_t id;
    if (nargs == 1) {
        id = getpid();
    } else if (nargs == 2) {
        id = atoi(args[1]);
    } else {
        fprintf(stderr, "pinfo: too many arguments\n");
        return TSK_FAIL;
    }

    char proc_file[PATH_MAX];
    struct stat proc_stat;
    sprintf(proc_file, "/proc/%d", id);

    if (stat(proc_file, &proc_stat) == -1) {
        fprintf(stderr, "sh: pinfo: invalid pid\n");
        return TSK_FAIL;
    }

    char proc_file_status[PATH_MAX];
    strcpy(proc_file_status, proc_file);
    strcat(proc_file_status, "/stat");

    FILE *proc_id_status = fopen(proc_file_status, "r");
    if (proc_id_status == NULL) {
        fprintf(stderr, "sh: pinfo: failed to open %s\n", proc_file_status);
        return TSK_FAIL;
    }

    pid_t store_id;
    char comm[PATH_MAX];
    char state;

    fscanf(proc_id_status, "%d %s %c", &store_id, comm, &state);

    long long unsigned sh_null;
    for (int i = 4; i <= 22; i++)
        fscanf(proc_id_status, "%llu", &sh_null);
    
    long unsigned vsize;
    fscanf(proc_id_status, "%lu", &vsize);

    char proc_exe[PATH_MAX];
    strcpy(proc_exe, proc_file);
    strcat(proc_exe, "/exe");

    char exe_path[PATH_MAX];
    memset(exe_path, 0, PATH_MAX);
    if (readlink(proc_exe, exe_path, PATH_MAX) == -1) {
        perror("sh: readlink");
        return TSK_FAIL;
    }
    rel_path(exe_path);

    printf("pid : %d\n", id);
    printf("state : %c%c\n", state, (tcgetpgrp(STDIN_FILENO) == getpgid(store_id)) ? '+' : '\0');
    printf("memory : %lu {Virtual Memory}\n", vsize);
    printf("executable Path : %s\n", relpath);

    fclose(proc_id_status);
    return TSK_OK;
}