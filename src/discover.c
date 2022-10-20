#include "../include/global.h"
#include "../include/discover.h"
#include "../include/utils.h"

int disc_es;
char files_only = 0;
char dirs_only = 0;

extern int alphasort();
int file_select_no_r(const struct dirent *dr) {
    if (strcmp(dr->d_name, ".") == 0 || strcmp(dr->d_name, "..") == 0)
        return 0;
    return 1;
}

void sh_discover_r(char *path, char *search_for) {
    struct dirent **files;
    int ndirs = scandir(path, &files, file_select_no_r, alphasort);

    if (ndirs == -1) {
        disc_es = TSK_FAIL;
        perror(path);
        return;
    }

    for (int i = 0; i < ndirs; i++) {
        char *next_path = join_path(path, files[i]->d_name);

        if (search_for == NULL) {
            if (files_only && !dirs_only) {
                if (files[i]->d_type == DT_REG)
                    printf("%s\n", next_path);
            } else if (!files_only && dirs_only) {
                if (files[i]->d_type == DT_DIR)
                    printf("%s\n", next_path);
            } else {
                printf("%s\n", next_path);
            }
        } else {
            if (files_only && !dirs_only) {
                if (files[i]->d_type == DT_REG)
                    if (strcmp(files[i]->d_name, search_for) == 0) {
                        printf("%s\n", next_path);
                    }
            } else if (!files_only && dirs_only) {
                // if (files[i]->d_type == DT_DIR)
                //     if (strcmp(files[i]->d_name, search_for) == 0) {
                //         printf("%s\n", next_path);
                //     }
            } else {
                if (strcmp(files[i]->d_name, search_for) == 0 && files[i]->d_type != DT_DIR) {
                    printf("%s\n", next_path);
                }
            }
        }

        if (files[i]->d_type == DT_DIR) {
            sh_discover_r(next_path, search_for);
        }

        free(next_path);
    }

    for (int i = 0; i < ndirs; i++)
        free(files[i]);
    free(files);
}

int sh_discover(int nargs, char *args[]) {
    disc_es = TSK_OK;
    files_only = dirs_only = 0;

    char search = 0;
    char *search_for = NULL;

    int dirs = 0;
    for (int i = 1; i < nargs; i++) {
        char *arg = args[i];
        if (arg[0] == '-') {
            for (int j = 1; j < strlen(arg); j++) {
                if (arg[j] == 'd')
                    dirs_only = 1;
                else if (arg[j] == 'f')
                    files_only = 1;
                else {
                    fprintf(stderr, "discover: invalid flag -%c\n", arg[j]);
                    disc_es = TSK_FAIL;
                    return TSK_FAIL;
                }
            }
        } else if (arg[0] == '"') {
            if (search == 1 || arg[strlen(arg) - 1] != '"') {
                /* invalid args */
                fprintf(stderr, "discover: invalid args\n");
                return TSK_FAIL;
            }
            
            arg[strlen(arg) - 1] = '\0';
            search_for = arg + 1;

            search = 1;
        } else {
            dirs++;
        }
    }

    char dot[2] = ".";
    if (dirs == 0) {
        args[nargs++] = dot;
        if (search) {
            // if (strcmp(search_for, dot) == 0) {
            //     if (!(files_only && !dirs_only))
            //         printf("%s\n", dot);
            // }
        } else  {
            if (!(files_only && !dirs_only))
                printf("%s\n", dot);
        }
    }

    for (int i = 1; i < nargs; i++) {
        char *arg = args[i];
        if (arg[0] == '-')
            continue;
        
        if (arg[0] == '"')
            continue;

        char after_tilde[PATH_MAX];
        memset(after_tilde, 0, sizeof(after_tilde));
        if (arg[0] == '~') {
            strcat(after_tilde, homedir);
            strcat(after_tilde, arg + 1);
        } else {
            strcat(after_tilde, arg);
        }

        if (search)
            sh_discover_r(after_tilde, search_for);
        else
            sh_discover_r(after_tilde, NULL);        
    }

    return disc_es;
}