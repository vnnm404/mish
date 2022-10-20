#include "../include/global.h"
#include "../include/prompt.h"
#include "../include/utils.h"
#include "../include/history.h"
#include "../include/list.h"

/* prompt colors */
char *res_color = KNRM;
char *pri_color = KGRN;
char *sec_color = KBLU;

/* exit stat colors */
char *succ_color = KGRN;
char *fail_color = KRED;

/* ls colors */
char *dir_color = KBLU;
char *exe_color = KGRN;
char *sym_color = KCYN;

struct sh_proc_list *sh_procs;
struct sig_msgs *sh_msgs;

int sh_id;

int dstdin;
int dstdout;
int redirect_in;
int redirect_out;

int next_job_id;
int last_exit_stat;
long last_time_taken;

char cmd[CMD_MAX];
char cmd_copy[CMD_MAX];
char homedir[2 * HOSTNAME_MAX];
char relpath[PATH_MAX];

char error_buf[PATH_MAX];

char cwd[PATH_MAX];
char oldwd[PATH_MAX];

struct termios old_term;
struct termios new_term;

void init();
void handle_msgs();
void killall_bg();

void z_handle_sig();
void c_handle_sig();

int main(int argc, char *argv) {
    init();

    while (1) {
        if (getcwd(cwd, PATH_MAX) == NULL) {
            perror("getcwd");
            exit_gracefully(1);
        }
        rel_path(cwd);

        handle_msgs();
        if (sh_procs->nprocs == 0) {
            next_job_id = 1;
        }

        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1) {
            perror("sh: tcsetattr");
            exit_gracefully(1);
        }

        prompt(last_exit_stat, last_time_taken, 1);

        if (tcsetattr(STDIN_FILENO, TCSANOW, &old_term) == -1) {
            perror("sh: tcsetattr");
            exit_gracefully(1);
        }

        fflush(stdout);

        strcpy(cmd_copy, cmd);
        if (strlen(cmd_copy) != 0)
            add_to_history();

        int status = parse_cmd();
        fflush(stdout);

        if (status == SH_END) {
            if (sh_procs->nprocs != 0) {
                if (sh_procs->nprocs == 1) {
                    printf("%d task is still running\n", sh_procs->nprocs);
                    printf("kill it and exit? [y/n]: ");
                } else {
                    printf("%d tasks are still running\n", sh_procs->nprocs);
                    printf("kill all and exit? [y/n]: ");
                }

                char rsp[CMD_MAX];
                fgets(rsp, CMD_MAX, stdin);

                if (rpmatch(rsp)) {
                    killall_bg();
                    break;
                }
            } else
                break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

    fclose(histfp);
    sig_msgs_free(sh_msgs);
    sh_proc_list_free(sh_procs);
    return 0;
}

void handle_sig(int sig, siginfo_t *info, void *vp) {
	// printf("Process ended\n");
    int wstat;
    int chid;
    char msg[PATH_MAX];
    while ((chid = waitpid(-1, &wstat, WUNTRACED | WNOHANG)) > 0) {
        char *name = sh_proc_list_get_name(sh_procs, chid);
        if (name == NULL) {
            fprintf(stderr, "sh: proc_list: name not found in child processes (wtf)\n");
            exit_gracefully(1);
        }

        if (WIFEXITED(wstat)) {
            int wes = WEXITSTATUS(wstat);
            if (wes == 0)
                sprintf(msg, "%s(%d)%s Ended (normally) %d\t%s\n", succ_color, (last_exit_stat = WEXITSTATUS(wstat)), res_color, chid, name);
            else
                sprintf(msg, "%s(%d)%s Ended (abnormally) %d\t%s\n", fail_color, (last_exit_stat = WEXITSTATUS(wstat)), res_color, chid, name);
            sh_proc_list_rem(sh_procs, chid);
        }
        else if (WIFSIGNALED(wstat)) {
            sprintf(msg, "%s(%d)%s Killed (abnormally) %d\t%s\n", fail_color, WTERMSIG(wstat), res_color, chid, name);
            sh_proc_list_rem(sh_procs, chid);
        }
        else if (WIFSTOPPED(wstat)) {
            sprintf(msg, "(%d) Stopped %d\t%s\n", WSTOPSIG(wstat), chid, name);

            struct sh_proc *p = sh_procs->head->next;
            while (p != NULL) {
                if (p->id == chid) {
                    p->stopped = 1;
                    break;
                }

                p = p->next;
            }
        }

        struct sh_msg *m = sh_msg_init(msg);
        sig_msgs_add(sh_msgs, m);
    }

    if (sh_procs->nprocs == 0) {
        next_job_id = 1;
    }
}

void z_handle_sig() {
    int fg_p = tcgetpgrp(dstdin);
    if (fg_p == getpgid(sh_id)) {
        return;
    }

    int id = getpid();
    if (kill(id, SIGSTOP) == -1) {
        perror("sh: kill");
        exit(1);
    }
}

void c_handle_sig() {
    int fg_p = tcgetpgrp(dstdin);
    if (fg_p == getpgid(sh_id)) {
        return;
    }

    if (kill(getpid(), SIGKILL) == -1) {
        perror("sh: kill");
        exit(1);
    }
}

void init() {
    /* terminal control setup */
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1) {
        perror("sh: ctty");
        exit_gracefully(1);
    }
    sh_id = getpid();

    /* termios */
    if (tcgetattr(STDIN_FILENO, &old_term) == -1) {
        perror("sh: tcgetattr");
        exit_gracefully(1);
    }

    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1) {
        perror("sh: tcsetattr");
        exit_gracefully(1);
    }

    last_exit_stat = 0;
    last_time_taken = 0;
    next_job_id = 0;

    dstdin = dup(STDIN_FILENO);
    dstdout = dup(STDOUT_FILENO);
    redirect_in = STDIN_FILENO;
    redirect_out = STDOUT_FILENO;

    /* sig handlers */
    struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_flags = SA_SIGINFO | SA_RESTART;
	act.sa_sigaction = handle_sig;
	
	struct sigaction oldact;
	sigaction(SIGCHLD, &act, &oldact);

    if (signal(SIGTSTP, &z_handle_sig) == SIG_ERR) {
        perror("sh: signal");
        exit_gracefully(1);
    }

    if (signal(SIGINT, &c_handle_sig) == SIG_ERR) {
        perror("sh: signal");
        exit_gracefully(1);
    }

    /* cd setup */
    getcwd(homedir, PATH_MAX);

    strcpy(oldwd, "~/~");

    /* history setup */
    histfp = fopen(".sh_history", "ra+");
    realpath(".sh_history", history_file_name);
    if (histfp == NULL) {
        histfp = fopen(history_file_name, "w+");

        if (histfp == NULL) {
            fprintf(stderr, "%s: failed to open / create", history_file_name);
            exit_gracefully(1);
        }
    }
    fseek(histfp, 0, SEEK_SET);

    /* other */
    sh_msgs = sig_msgs_init();
    sh_procs = sh_proc_list_init();
}

void handle_msgs() {
    while (sh_msgs->nmsgs > 0) {
        printf("%s", sh_msgs->head->next->msg);
        sig_msgs_rem(sh_msgs);
    }
}

void killall_bg() {
    for (struct sh_proc *p = sh_procs->head->next; p != NULL; p = p->next) {
        if (kill(-p->id, SIGKILL) == -1) {
            perror("kill");
            exit_gracefully(1);
        }
    }
}

void exit_gracefully(int st) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

    fclose(histfp);
    sig_msgs_free(sh_msgs);
    sh_proc_list_free(sh_procs);

    exit(st);
}