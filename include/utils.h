#ifndef _UTILS_H_
#define _UTILS_H_

char* trim(char *str);
int rel_path(char *path);
int parse_cmd();
int run_task(char *task, char type);
int perm_str(mode_t md, char isdir, char *str);
long micro_timestamp();
char *join_path(const char *pref, const char *base);
long get_width(long x);
long max(long a, long b);
int pipe_parse(char *task, char type);

int run_task_in_foreground(int nargs, char *args[]);
int run_task_in_background(int nargs, char *args[]);

#endif /* _UTILS_H_ */