#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "../include/list.h"

#define LS_BLOCK_RATIO          2

#define CMD_MAX                 4096
#define PATH_MAX                4096
#define HOSTNAME_MAX            256
#define TIME_MAX                4096
#define PROMPT_MAX              PATH_MAX + HOSTNAME_MAX
#define BG_MAX                  512

#define TASK_FG 0
#define TASK_BG 1

#define TNRM  "\x1b[0m"
#define TBLD  "\x1b[1m"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KPRP  "\x1B[0;35m"
#define KPNK  "\x1B[38;5;213m"
#define KBGRN "\x1B[42m"

/* prompt colors */
extern char *res_color;
extern char *pri_color;
extern char *sec_color;

/* exit stat colors */
extern char *succ_color;
extern char *fail_color;

/* ls colors */
extern char *dir_color;
extern char *exe_color;
extern char *sym_color;

enum sh_status {
    SH_OK,
    SH_END,
    SH_BADCMD
};

enum tsk_status {
    TSK_OK,
    TSK_FAIL,
    TSK_EXIT
};

extern struct sh_proc_list *sh_procs;
extern struct sig_msgs *sh_msgs;

extern void z_handle_sig();
extern void c_handle_sig();

extern int sh_id;

extern int dstdin;
extern int dstdout;
extern int redirect_in;
extern int redirect_out;

extern int next_job_id;
extern int last_exit_stat;
extern long last_time_taken;

extern char cmd[];
extern char cmd_copy[];
extern char homedir[];
extern char relpath[];

extern char cwd[];
extern char oldwd[];

extern struct termios old_term;
extern struct termios new_term;

void exit_gracefully(int st);

#endif /* _GLOBAL_H_ */