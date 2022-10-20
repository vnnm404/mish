#ifndef _LIST_H_
#define _LIST_H_

struct sh_msg {
    char *msg;
    struct sh_msg *next;
};

struct sig_msgs {
    int nmsgs;
    struct sh_msg *head;
};

int sig_msgs_free(struct sig_msgs *sm);
int sig_msgs_rem(struct sig_msgs *sm);
int sig_msgs_add(struct sig_msgs *sm, struct sh_msg *m);
struct sig_msgs* sig_msgs_init();
struct sh_msg* sh_msg_init(char *msg);

struct sh_proc {
    int jid;
    pid_t id;
    pid_t gid;
    char stopped;
    char terminated;
    char *name;
    struct sh_proc *next;
};

struct sh_proc_list {
    int nprocs;
    struct sh_proc *head;
};

int sh_proc_list_free(struct sh_proc_list *l);
int sh_proc_list_rem(struct sh_proc_list *l, pid_t id);
int sh_proc_list_add(struct sh_proc_list *l, struct sh_proc *shp);
struct sh_proc_list* sh_proc_list_init();
struct sh_proc* sh_proc_init(pid_t id, char *args[]);
char* sh_proc_list_get_name(struct sh_proc_list *l, pid_t id);

#endif /* _LIST_H_ */