#include "../include/global.h"
#include "../include/utils.h"
#include "../include/core.h"
#include "../include/history.h"
#include "../include/discover.h"
#include "../include/pinfo.h"
#include "../include/jobs.h"
#include "../include/sig.h"
#include "../include/fg.h"
#include "../include/bg.h"

long max(long a, long b) {
    return a > b ? a : b;
}

char* trim(char *str) {
    if (str == NULL)
        return str;

    char *end = &str[strlen(str) - 1];

    while (isspace(*str))
        str++;
    
    while (isspace(*end))
        end--;
    end[1] = '\0';

    return str;
}

char* join_path(const char *pref, const char *base)
{
    long len_pref = strlen(pref);
    long len_base = strlen(base);

    char *joined = (char *)calloc((len_pref + len_base + 2), sizeof(char));
    strncpy(joined, pref, len_pref);
    joined[len_pref] = '/';
    strcat(joined, base);

    return joined;
}

int rel_path(char *path) {
    char *save_path = path;
    char *hdir = homedir;
    while (*path != '\0' && *hdir != '\0' && *hdir == *path)
        hdir++, path++;
    
    if (*hdir == '\0' && (*path == '/' || *path == '\0')) {
        strcpy(relpath, "~");
        strcat(relpath, path);
    } else {
        strcpy(relpath, save_path);
    }

    return 0;
}

long get_width(long n) {
    if (n == 0)
        return 1;
    long w = 0;
    if (n < 0)
        w = 1, n = abs(n);
    while (n > 0) {
        w++;
        n /= 10;
    }
    return w;
}

long micro_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

int perm_str(mode_t md, char isdir, char *str) {
    for (int i = 0; i < 10; i++)
        str[i] = '-';

    if (isdir)
        str[0] = 'd';
    
    if (md & S_IRUSR) str[1] = 'r';
    if (md & S_IWUSR) str[2] = 'w';
    if (md & S_IXUSR) str[3] = 'x';

    if (md & S_IRGRP) str[4] = 'r';
    if (md & S_IWGRP) str[5] = 'w';
    if (md & S_IXGRP) str[6] = 'x';

    if (md & S_IROTH) str[7] = 'r';
    if (md & S_IWOTH) str[8] = 'w';
    if (md & S_IXOTH) str[9] = 'x';

    return 0;
}

/* main tokenisation */
int parse_cmd() {
    char *tcmd = trim(cmd);
    strcat(tcmd, ";");

    char *l = tcmd, *r = tcmd;
    while (*r != '\0') {
        switch (*r) {
            case ';': {
                *r = '\0';
                /* l now points to the command */

                if (strlen(l) != 0) {
                    // printf("; ||| %s\n", l);
                    int rt = pipe_parse(l, TASK_FG);
                    if (rt == TSK_EXIT)
                        return SH_END;
                    
                    if (rt == TSK_FAIL)
                        last_exit_stat = 1;
                }

                /* reset l for the next command */
                l = r + 1;
                break;
            }
            case '&': {
                *r = '\0';
                /* l now points to the command */

                if (strlen(l) != 0) {
                    // printf("& ||| %s\n", l);
                    int rt = pipe_parse(l, TASK_BG);
                    if (rt == TSK_EXIT)
                        return SH_END;
                    
                    if (rt == TSK_FAIL)
                        last_exit_stat = 1;
                }

                /* reset l for the next command */
                l = r + 1;
                break;
            }
            default:
                break;
        }

        r++;
    }

    return SH_OK;
}

int pipe_parse(char *task, char type) {
    int iv = 0;
    char *indv[CMD_MAX];
    char *l = task, *r = task;
    while (*r != '\0') {
        if (*r == '|') {
            *r = '\0';
            indv[iv++] = l;
            l = r + 1;
        }
        r++;
    }
    indv[iv++] = l;

    if (iv == 1) {
        return run_task(indv[0], type);
    }

    int pipe_in[2], pipe_out[2];
    pipe_in[0] = dstdin;

    for (int i = 0; i < iv - 1; i++) {
        if (pipe(pipe_out) == -1) {
            perror("sh: pipe");
            exit_gracefully(1);
        }

        redirect_in = pipe_in[0];
        redirect_out = pipe_out[1];

        int rt = run_task(indv[i], type);
        if (rt == TSK_EXIT)
            return TSK_EXIT;
        
        close(pipe_out[1]);

        pipe_in[0] = pipe_out[0];
    }

    redirect_in = pipe_in[0];
    redirect_out = dstdout;

    int rt = run_task(indv[iv - 1], type);
    if (rt == TSK_EXIT)
        return TSK_EXIT;

    return TSK_OK;
}

int run_task(char *task, char type) {
    /* redirection and pipe parsing */
    /* first we attempt redirection parsing */
    /* because multiple redirection doesn't exist, we can check for < and > directly */
    /* we will only parse valid commands */
    /* any invalid command will lead to undefined behaviour */

    char flag_append = 0;
    char *base = task, *i_red = NULL, *o_red = NULL;
    char *r = task;
    while (*r != '\0') {
        switch(*r) {
            case '<': {
                *r = '\0';

                i_red = r + 1;
                break;
            }
            case '>': {
                *r = '\0';
                if (*(r + 1) == '>') {
                    flag_append = 1;
                    r++;

                    *r = '\0';
                }

                o_red = r + 1;
                break;
            }
        }

        r++;
    }

    // printf("%s\n%s\n%s\n", base, i_red, o_red);
    // return TSK_OK;

    i_red = trim(i_red);
    o_red = trim(o_red);

    if (i_red != NULL) {
        if ((redirect_in = open(i_red, O_RDONLY)) == -1) {
            perror("parser: open");
            return TSK_FAIL;
        }
    }

    if (flag_append) {
        if (o_red != NULL && (redirect_out = open(o_red, O_RDWR | O_CREAT | O_APPEND, 00644)) == -1) {
            perror("parser: open");
            return TSK_FAIL;
        }
    } else {
        if (o_red != NULL && (redirect_out = open(o_red, O_RDWR | O_CREAT | O_TRUNC, 00644)) == -1) {
            perror("parser: open");
            return TSK_FAIL;
        }
    }

    int nargs = 0;
    char *args[CMD_MAX];
    memset(args, 0, sizeof(args));

    char *token = strtok(base, " \t\n");
    while (token != NULL) {
        args[nargs++] = token;

        token = strtok(NULL, " \t\n");
    }

    if (args[0] == NULL)
        return 0;

    char after_tilde[PATH_MAX];
    memset(after_tilde, 0, sizeof(after_tilde));
    if (args[0][0] == '~') {
        strcat(after_tilde, homedir);
        strcat(after_tilde, args[0] + 1);

        args[0] = after_tilde;
    }

    int tsk_stat = -1;
    if (strcmp(args[0], "exit") == 0) {
        return TSK_EXIT;
    }

    dup2(redirect_in, STDIN_FILENO);
    dup2(redirect_out, STDOUT_FILENO);
    
    if (strcmp(args[0], "pwd") == 0)
        tsk_stat = sh_pwd(nargs, args);
    
    else if (strcmp(args[0], "cd") == 0)
        tsk_stat = sh_cd(nargs, args);

    else if (strcmp(args[0], "echo") == 0)
        tsk_stat = sh_echo(nargs, args);

    else if (strcmp(args[0], "ls") == 0)
        tsk_stat = sh_ls(nargs, args);

    else if (strcmp(args[0], "history") == 0)
        tsk_stat = sh_history(nargs, args);

    else if (strcmp(args[0], "discover") == 0)
        tsk_stat = sh_discover(nargs, args);

    else if (strcmp(args[0], "pinfo") == 0)
        tsk_stat = sh_pinfo(nargs, args);

    else if (strcmp(args[0], "jobs") == 0)
        tsk_stat = sh_jobs(nargs, args);

    else if (strcmp(args[0], "sig") == 0)
        tsk_stat = sh_sig(nargs, args);

    else if (strcmp(args[0], "fg") == 0)
        tsk_stat = sh_fg(nargs, args);
    
    else if (strcmp(args[0], "bg") == 0)
        tsk_stat = sh_bg(nargs, args);

    dup2(dstdin, STDIN_FILENO);
    dup2(dstdout, STDOUT_FILENO);

    if (tsk_stat == -1) {
        if (type == TASK_FG)
            tsk_stat = run_task_in_foreground(nargs, args);
        else if (type == TASK_BG)
            tsk_stat = run_task_in_background(nargs, args);
    }

    redirect_in = STDIN_FILENO;
    redirect_out = STDOUT_FILENO;

    return tsk_stat;
}

int run_task_in_foreground(int nargs, char *args[]) {
    int pchld = fork();

    if (pchld == -1) {
        perror("sh: fork");
        return TSK_FAIL;
    }

    long start = micro_timestamp();

    if (pchld == 0) {
        signal(SIGINT, &c_handle_sig);
        signal(SIGTSTP, &z_handle_sig);

        dup2(redirect_in, STDIN_FILENO);
        dup2(redirect_out, STDOUT_FILENO);

        setpgid(0, 0);

        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(1);
        }
    } else {
        struct sh_proc *shp = sh_proc_init(pchld, args);
        sh_proc_list_add(sh_procs, shp);

        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        tcsetpgrp(STDIN_FILENO, pchld);

        int wstat;
        // struct rusage fg_rusage;
        // if (wait4(pchld, &wstat, WUNTRACED, &fg_rusage) == -1) {
		if (waitpid(pchld, &wstat, WUNTRACED) == -1) {
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
            if (sh_proc_list_rem(sh_procs, pchld) == 2) {
                fprintf(stderr, "sh: proc_list: failed to find child (wtf)\n");
                exit_gracefully(1);
            }
        } else if (WIFSIGNALED(wstat)) {
            last_exit_stat = 1;
            if (sh_proc_list_rem(sh_procs, pchld) == 2) {
                fprintf(stderr, "sh: proc_list: failed to find child (wtf)\n");
                exit_gracefully(1);
            }
        } else if (WIFSTOPPED(wstat)) {
            /* handle ? */
            printf("Stopped\n");
        }
    }

    return TSK_OK;
}

int run_task_in_background(int nargs, char *args[]) {
    int pchld = fork();

    if (pchld == -1) {
        perror("sh: fork");
        return TSK_FAIL;
    }

    if (pchld == 0) {
        signal(SIGTSTP, &z_handle_sig);
        signal(SIGINT, &c_handle_sig);

        dup2(redirect_in, STDIN_FILENO);
        dup2(redirect_out, STDOUT_FILENO);

        setpgid(0, 0);

        if (execvp(args[0], args) == -1) {
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(1);
        }
    } else {
        struct sh_proc *shp = sh_proc_init(pchld, args);
        sh_proc_list_add(sh_procs, shp);

        /* idk man */
        printf("[%d] %d\n", shp->jid, pchld);
        // sleep(1);
    }

    return TSK_OK;
}
