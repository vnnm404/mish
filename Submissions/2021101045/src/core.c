#include "../include/global.h"
#include "../include/core.h"
#include "../include/utils.h"

extern int alphasort();
int ncase_alphasort(const struct dirent **f1, const struct dirent **f2) {
    return strcasecmp((*f1)->d_name, (*f2)->d_name);
}

int file_select_minimal(const struct dirent *dr) {
    if (strcmp(dr->d_name, ".") == 0 || strcmp(dr->d_name, "..") == 0 || dr->d_name[0] == '.')
        return 0;
    return 1;
}
int file_select_all(const struct dirent *dr) {
    return 1;
}

int sh_pwd(int nargs, char *args[]) {
    if (nargs > 1) {
        fprintf(stderr, "pwd: too many arguments\n");
        return TSK_FAIL;
    }
    printf("%s\n", cwd);
    return TSK_OK;
}

int sh_cd(int nargs, char *args[]) {
    if (nargs > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return TSK_FAIL;
    }

    if (nargs == 1) {
        if (chdir(homedir) == -1) {
            /* not sure why i bothered */
            perror(homedir);
            return TSK_FAIL;
        }

        return TSK_OK;
    }

    char *path = args[1];

    if (strcmp(path, "-") == 0) {
        if (strcmp(oldwd, "~/~") == 0) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return TSK_FAIL;
        }

        if (chdir(oldwd) == -1) {
            perror(oldwd);
            return TSK_FAIL;
        }

        getcwd(cwd, PATH_MAX);

        printf("%s\n", cwd);

        /* note cwd, is now outdated */
        strcpy(oldwd, cwd);
        return TSK_OK;
    }    

    char after_tilde[PATH_MAX];
    memset(after_tilde, 0, sizeof(after_tilde));
    if (path[0] == '~') {
        strcat(after_tilde, homedir);
        strcat(after_tilde, path + 1);
    } else {
        strcat(after_tilde, path);
    }

    if (chdir(after_tilde) == -1) {
        perror(after_tilde);
        return TSK_FAIL;
    }

    /* update oldwd only if new wd is valid */
    strcpy(oldwd, cwd);
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("getcwd");
        exit_gracefully(1);
    }

    return TSK_OK;
}

int sh_echo(int nargs, char *args[]) {
    for (int i = 1; i < nargs; i++)
        printf("%s ", args[i]);
    printf("\n");

    return TSK_OK;
}

int sh_ls(int nargs, char *args[]) {
    char multiple = 0;
    char list_all = 0;
    char details = 0;

    int es = TSK_OK;

    int outputs = 0;
    for (int i = 1; i < nargs; i++) {
        char *arg = args[i];
        
        if (arg[0] == '-') {
            arg++;
            while (*arg != '\0') {
                if (*arg == 'a')
                    list_all = 1;
                else if (*arg == 'l')
                    details = 1;
                else {
                    fprintf(stderr, "ls: invalid flag -%c\n", *arg);
                    return TSK_FAIL;
                }

                arg++;
            }
        } else
            outputs++;
    }

    if (outputs > 1)
        multiple = 1;

    if (outputs == 0)
        args[nargs++] = cwd;

    
    for (int i = 1; i < nargs; i++) {
        char *arg = args[i];
        
        if (arg[0] == '-')
            continue;

        char after_tilde[PATH_MAX];
        memset(after_tilde, 0, sizeof(after_tilde));
        if (arg[0] == '~') {
            strcat(after_tilde, homedir);
            strcat(after_tilde, arg + 1);

            arg = after_tilde;
        }

        char isdir = 1;

        int ndirs;
        struct dirent **files;

        struct stat sym_deals;
        if (lstat(arg, &sym_deals) == -1) {
            perror(arg);
            es = TSK_FAIL;
            continue;
        } else {
            if (S_ISLNK(sym_deals.st_mode)) {
                isdir = 0;
            } else {
                if (list_all)
                    ndirs = scandir(arg, &files, file_select_all, ncase_alphasort);
                else
                    ndirs = scandir(arg, &files, file_select_minimal, ncase_alphasort);
            }
        }

        if (ndirs == -1) {
            if (errno == ENOTDIR) {
                isdir = 0;
            } else {
                perror(arg);

                es = TSK_FAIL;

                if (outputs > 1)
                    printf("\n");

                continue;
            }
        }
        
        if (isdir) {
            if (multiple) {
                printf("%s:\n", arg);
            }

            long max_widths[5];
            memset(max_widths, 0, sizeof(max_widths));
            max_widths[0] = 10;

            if (details) {
                struct stat arg_dets;
                if (stat(arg, &arg_dets) == -1) {
                    perror(arg);
    
                    if (outputs > 1)
                        printf("\n");

                    return TSK_FAIL;
                }

                long total = 0;
                char joined_path[PATH_MAX];
                for (int j = 0; j < ndirs; j++) {
                    strcpy(joined_path, arg);
                    strcat(joined_path, "/");
                    strcat(joined_path, files[j]->d_name);

                    struct stat file_dets;
                    if (stat(joined_path, &file_dets) == -1) {
                        perror(files[j]->d_name);
                        return TSK_FAIL;
                    }

                    struct passwd *file_owner_pwd = getpwuid(file_dets.st_uid);
                    struct passwd *file_group_pwd = getpwuid(file_dets.st_gid);

                    max_widths[1] = max(max_widths[1], get_width(file_dets.st_nlink));
                    max_widths[2] = max(max_widths[2], strlen(file_owner_pwd->pw_name));
                    max_widths[3] = max(max_widths[3], strlen(file_group_pwd->pw_name));
                    max_widths[4] = max(max_widths[4], get_width(file_dets.st_size));

                    total += file_dets.st_blocks / LS_BLOCK_RATIO;
                }

                printf("total %ld\n", total);
            }

            /* print the dets */
            char joined_path[PATH_MAX];
            for (int j = 0; j < ndirs; j++) {
                strcpy(joined_path, arg);
                strcat(joined_path, "/");
                strcat(joined_path, files[j]->d_name);

                struct stat file_dets;
                if (stat(joined_path, &file_dets) == -1) {
                    perror(files[j]->d_name);
                    return TSK_FAIL;
                }

                char pstr[12];
                memset(pstr, 0, sizeof(pstr));
                perm_str(file_dets.st_mode, files[j]->d_type == DT_DIR, pstr);

                struct passwd *file_owner_pwd = getpwuid(file_dets.st_uid);
                struct passwd *file_group_pwd = getpwuid(file_dets.st_gid);

                /* getting time stuff */
                struct tm *time_info = localtime(&file_dets.st_mtim.tv_sec);
                if (time_info == NULL) {
                    fprintf(stderr, "ls: localtime: failed\n");
                    return TSK_FAIL;
                }

                char time_str[TIME_MAX];
                strftime(time_str, TIME_MAX, "%b %e %02k:%02M", time_info); 

                /* final print */
                char *color = KNRM, *fix_color = KNRM;
                if (files[j]->d_type == DT_DIR) {
                    color = dir_color;
                } else if (files[j]->d_type == DT_LNK) {
                    color = sym_color;
                } else if (pstr[3] == 'x' || pstr[6] == 'x' || pstr[9] == 'x') {
                    color = exe_color;
                }

                if (details) {
                    printf("%s ", pstr);

                    int k = max_widths[1] - get_width(file_dets.st_nlink);
                    while (k--)
                        printf(" ");
                    printf("%ld ", file_dets.st_nlink);

                    k = max_widths[2] - strlen(file_owner_pwd->pw_name);
                    while (k--)
                        printf(" ");
                    printf("%s ", file_owner_pwd->pw_name);

                    k = max_widths[3] - strlen(file_group_pwd->pw_name);
                    while (k--)
                        printf(" ");
                    printf("%s ", file_group_pwd->pw_name);

                    k = max_widths[4] - get_width(file_dets.st_size);
                    while (k--)
                        printf(" ");
                    printf("%ld %s %s%s%s", file_dets.st_size,
                            time_str,
                            color, files[j]->d_name, fix_color);

                    if (files[j]->d_type == DT_LNK) {
                        char rp[PATH_MAX];
                        memset(rp, 0, sizeof(rp));
                        if (readlink(joined_path, rp, PATH_MAX) == -1) {
                            perror("readlink");
                            return TSK_FAIL;
                        }
                        printf(" -> %s%s%s\n", color, rp, fix_color);
                    } else
                        printf("\n");
                } else
                    printf("%s%s%s ", color, files[j]->d_name, fix_color);
            }

            if (!details)
                printf("\n");

            for (int i = 0; i < ndirs; i++)
                free(files[i]);
            free(files);
        } else {
            struct stat file_dets;
            if (stat(arg, &file_dets) == -1) {
                perror(arg);

                if (outputs > 1)
                    printf("\n");

                es = TSK_FAIL;

                continue;
            }

            char pstr[12];
            memset(pstr, 0, sizeof(pstr));
            perm_str(file_dets.st_mode, 0, pstr);

            char *color = KNRM, *fix_color = KNRM;
            if (S_ISLNK(sym_deals.st_mode)) {
                color = sym_color;
            } else if (pstr[3] == 'x' || pstr[6] == 'x' || pstr[9] == 'x') {
                color = exe_color;
            }

            struct passwd *file_owner_pwd = getpwuid(file_dets.st_uid);
            struct passwd *file_group_pwd = getpwuid(file_dets.st_gid);

            /* getting time stuff */
            struct tm *time_info = localtime(&file_dets.st_mtim.tv_sec);
            if (time_info == NULL) {
                fprintf(stderr, "ls: localtime: failed\n");
                return TSK_FAIL;
            }

            char time_str[TIME_MAX];
            strftime(time_str, TIME_MAX, "%b %e %k:%M", time_info); 

            if (details) {
                printf("%s %ld %s %s %ld %s %s%s%s", 
                    pstr,
                    file_dets.st_nlink,
                    file_owner_pwd->pw_name,
                    file_group_pwd->pw_name,
                    file_dets.st_size,
                    time_str,
                    color,
                    arg,
                    fix_color);

                if (S_ISLNK(sym_deals.st_mode)) {
                    char rp[PATH_MAX];
                    memset(rp, 0, sizeof(rp));
                    if (readlink(arg, rp, PATH_MAX) == -1) {
                        perror("readlink");
                        return TSK_FAIL;
                    }
                    printf(" -> %s%s%s\n", color, rp, fix_color);
                } else
                    printf("\n");
            } else
                printf("%s%s%s\n", color, arg, fix_color);
        }

        if (outputs > 1)
            printf("\n");

        outputs--;
    }

    return es;
}