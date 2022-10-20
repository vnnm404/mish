#include "../include/global.h"
#include "../include/list.h"

struct sh_proc* sh_proc_init(pid_t id, char *args[]) {
    struct sh_proc *shp = malloc(sizeof(struct sh_proc));
    if (shp == NULL) {
        fprintf(stderr, "sh: malloc: bad_alloc\n");
        exit_gracefully(1);
    }

    shp->jid = next_job_id++;
    shp->id = id;
    shp->stopped = 0;
    shp->terminated = 0;
    if (id < 0)
        shp->gid = -1;
    else
        shp->gid = getpgid(id);
    shp->terminated = shp->stopped = 0;

    char buf[PATH_MAX];
    memset(buf, 0, PATH_MAX);
    for (int i = 0; args[i] != NULL; i++) {
        strcat(buf, args[i]);
        strcat(buf, " ");
    }

    shp->name = malloc(strlen(buf) + 1);
    if (shp->name == NULL) {
        fprintf(stderr, "sh: malloc: bad_alloc\n");
        exit_gracefully(1);
    }

    strcpy(shp->name, buf);

    shp->next = NULL;

    return shp;
}

struct sh_proc_list* sh_proc_list_init() {
    struct sh_proc_list *l = malloc(sizeof(struct sh_proc_list));
    if (l == NULL) {
        fprintf(stderr, "sh: malloc: bad_alloc\n");
        exit_gracefully(1);
    }

    char *args[] = {"HEAD", NULL};
    l->nprocs = 0;
    l->head = sh_proc_init(-1, args);

    return l;
}

int sh_proc_list_add(struct sh_proc_list *l, struct sh_proc *shp) {
    if (shp == NULL || l == NULL) {
        fprintf(stderr, "sh: sig_add: dumbass args\n");
        exit_gracefully(1);
    }

    shp->next = l->head->next;
    l->head->next = shp;

    l->nprocs++;
    return 0;
}

int sh_proc_list_rem(struct sh_proc_list *l, pid_t id) {
    if (l == NULL || id <= -1) {
        fprintf(stderr, "sh: sig_add: dumbass args\n");
        exit_gracefully(1);
    }

    if (l->head->next == NULL) {
        fprintf(stderr, "sh: sig_add: empty list\n");
        exit_gracefully(1);
    }

    struct sh_proc *shp = l->head;
    while (shp->next != NULL) {
        if (shp->next->id == id) {
            struct sh_proc *k = shp->next;

            shp->next = k->next;

            free(k->name);
            free(k);

            l->nprocs--;
            return 0;
        }

        shp = shp->next;
    }

    return 2; /* couldn't find what you were looking for */
}

char* sh_proc_list_get_name(struct sh_proc_list *l, pid_t id) {
    struct sh_proc *p = l->head->next;
    while (p != NULL) {
        if (p->id == id)
            return p->name;
        
        p = p->next;
    }
    
    return NULL;
}

int sh_proc_list_free(struct sh_proc_list *l) {
    if (l == NULL)
        return 1;
    
    struct sh_proc *shp = l->head;
    while (shp != NULL) {
        struct sh_proc *k = shp;
        shp = shp->next;

        free(k->name);
        free(k);
    }

    free(l);
    return 0;
}

struct sh_msg* sh_msg_init(char *msg) {
    struct sh_msg *m = malloc(sizeof(struct sh_msg));
    if (m == NULL) {
        fprintf(stderr, "sh: malloc: bad_alloc\n");
        exit_gracefully(1);
    }

    m->next = NULL;
    m->msg = malloc(strlen(msg) + 1);
    strcpy(m->msg, msg);

    return m;
}

struct sig_msgs* sig_msgs_init() {
    struct sig_msgs *sm = malloc(sizeof(struct sig_msgs));
    if (sm == NULL) {
        fprintf(stderr, "sh: malloc: bad_alloc\n");
        exit_gracefully(1);
    }

    sm->nmsgs = 0;
    sm->head = sh_msg_init("HEAD");

    return sm;
}

int sig_msgs_add(struct sig_msgs *sm, struct sh_msg *m) {
    if (sm == NULL || m == NULL) {
        fprintf(stderr, "sh: sig_add: dumbass args\n");
        exit_gracefully(1);
    }

    m->next = sm->head->next;
    sm->head->next = m;

    sm->nmsgs++;
    return 0;
}

int sig_msgs_rem(struct sig_msgs *sm) {
    if (sm == NULL) {
        fprintf(stderr, "sh: sig_add: dumbass args\n");
        exit_gracefully(1);
    }

    if (sm->head->next == NULL) {
        fprintf(stderr, "sh: sig_add: empty list\n");
        exit_gracefully(1);
    }

    struct sh_msg *m = sm->head->next;
    sm->head->next = m->next;

    free(m->msg);
    free(m);

    sm->nmsgs--;
    return 0;
}

int sig_msgs_free(struct sig_msgs *sm) {
    if (sm == NULL)
        return 1;
    
    struct sh_msg *m = sm->head;
    while (m != NULL) {
        struct sh_msg *k = m;
        m = m->next;

        free(k->msg);
        free(k);
    }

    free(sm);
    return 0;
}