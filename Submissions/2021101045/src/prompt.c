#include "../include/global.h"
#include "../include/prompt.h"

extern void killall_bg();

void prompt(int exit_stat, long took, char take_input)
{
    struct utsname sysinfo;
    if (uname(&sysinfo) == -1)
    {
        perror("uname");
        exit_gracefully(1);
    }

    uid_t uid = getuid();
    struct passwd *pwd = getpwuid(uid);
    if (pwd == NULL)
    {
        perror("pwd");
        exit_gracefully(1);
    }

    char prompt_str[PROMPT_MAX];
    if (exit_stat)
    {
        if (took >= 1000000)
        {
            sprintf(prompt_str, "%s[%d]%s [%s@%s %s][took %.2lfs]$ ",
                    fail_color, exit_stat, res_color,
                    pwd->pw_name,
                    sysinfo.sysname,
                    relpath, (double)took / 1000000);
        }
        else
        {
            sprintf(prompt_str, "%s[%d]%s [%s@%s %s]$ ",
                    fail_color, exit_stat, res_color,
                    pwd->pw_name,
                    sysinfo.sysname,
                    relpath);
        }
    }
    else
    {
        if (took >= 1000000)
        {
            sprintf(prompt_str, "%s[%d]%s [%s@%s %s][took %.2lfs]$ ",
                    succ_color, exit_stat, res_color,
                    pwd->pw_name,
                    sysinfo.sysname,
                    relpath, (double)took / 1000000);
        }
        else
        {
            sprintf(prompt_str, "%s[%d]%s [%s@%s %s]$ ",
                    succ_color, exit_stat, res_color,
                    pwd->pw_name,
                    sysinfo.sysname,
                    relpath);
        }
    }

    printf("%s", prompt_str);
    fflush(stdout);

    /* input */
    if (take_input)
        input();
    // fgets(cmd, CMD_MAX, stdin);

    // cmd[strlen(cmd) - 1] = '\0';
    last_exit_stat = 0;
    last_time_taken = 0;
}

void input()
{
    memset(cmd, 0, CMD_MAX);
    int p = 0;

    while (1)
    {
        char key;
        if (read(STDIN_FILENO, &key, 1) < 0)
        {
            perror("sh: read");
            exit_gracefully(1);
        }

        if (key == 27)
        {
            // check if input left in stdin
            struct pollfd pfd = {
                .fd = STDIN_FILENO,
                .events = POLLIN};

            if (poll(&pfd, 1, 0) == 1)
            {
                char ekey1, ekey2;
                read(STDIN_FILENO, &ekey1, 1);
                read(STDIN_FILENO, &ekey2, 1);

                // printf("Read Escape Key\n");
                // fflush(stdout);
            }
            else
            {
                // printf("Read standalone Escape\n");
                // fflush(stdout);
            }
        }
        else if (key == 127)
        {
            /* backspace implementation */
            if (p == 0)
            {
                continue;
            }

            cmd[--p] = '\0';
            printf("\033[1D");
            printf(" ");
            printf("\033[1D");
            fflush(stdout);
        }
        else if (key == 9)
        {
            /* tabs! autocompletion */
            printf("\033[s"); /* save cur position */
            fflush(stdout);

            int bb = p - 1;
            int q = p - 1;
            while (q >= 0 && (cmd[q] != ' ' && cmd[q] != '/'))
            {
                q--;
            }
            q++;

            while (bb >= 0 && cmd[bb] != ' ')
            {
                bb--;
            }
            bb++;

            char dir[PATH_MAX];
            memset(dir, 0, PATH_MAX);
            if (bb == q)
            {
                strcpy(dir, cwd);
            }
            else
            {
                int dirp = 0;
                while (bb < q)
                {
                    dir[dirp++] = cmd[bb++];
                }
            }

            struct dirent **files;
            int nfiles = scandir(dir, &files, NULL, alphasort);
            if (nfiles == -1)
            {
                continue; /* comment this line for error handling */
                perror("sh: scandir");
                exit_gracefully(1);
            }

            int matches[nfiles];
            int mp = 0;
            for (int i = 0; i < nfiles; i++)
            {
                if (strcmp(".", files[i]->d_name) == 0 || strcmp("..", files[i]->d_name) == 0)
                {
                    continue;
                }

                int sq = q;
                int qq = 0;
                while (q < p)
                {
                    if (files[i]->d_name[qq] == '\0')
                    {
                        break;
                    }

                    if (cmd[q] != files[i]->d_name[qq])
                    {
                        break;
                    }
                    else
                    {
                        q++, qq++;
                    }
                }

                if (q == p)
                {
                    matches[mp++] = i;
                }

                q = sq;
            }

            if (mp == 1)
            {
                char *fp = files[matches[0]]->d_name;
                while (q < p)
                {
                    q++, fp++;
                }
                int qq = strlen(fp);
                p += qq;

                printf("\033[u"); /* restores cur position */
                if (files[matches[0]]->d_type == DT_DIR)
                {
                    printf("%s/", fp);
                    strcat(cmd, fp);
                    strcat(cmd, "/");
                    p += 1;
                }
                else
                {
                    printf("%s", fp);
                    strcat(cmd, fp);
                }
                fflush(stdout);
            }
            else if (mp > 1) {
                int matchp[nfiles];
                for (int i = 0; i < nfiles; i++)
                    matchp[i] = 0;

                for (int i = 0; i < mp; i++)
                {
                    int bruh = q;
                    // printf("starting\n");
                    // printf("\n%d - %d\n", strlen(files[matches[i]]->d_name), matchp[matches[i]]);
                    while (bruh < p && files[matches[i]]->d_name[matchp[matches[i]]] == cmd[bruh]) {
                        matchp[matches[i]]++;
                        bruh++;
                    }

                    // printf("\nerr: %c\n", files[matches[i]]->d_name[matchp[matches[i]]]);
                }

                int additions = 0;
                while (1) {
                    char c = files[matches[0]]->d_name[matchp[matches[0]]];
                    char br = 0;
                    for (int i = 1; i < mp; i++) {
                        if (c != files[matches[i]]->d_name[matchp[matches[i]]]) {
                            br = 1;
                            break;
                        }
                    }
                    if (br)
                        break;

                    additions++;
                    printf("%c", c);
                    cmd[p++] = c;
                    fflush(stdout);

                    for (int i = 0; i < mp; i++) {
                        matchp[matches[i]]++;
                    }
                }

                for (int i = 0; i < mp; i++)
                {
                    if (files[matches[i]]->d_type == DT_DIR)
                    {
                        printf("\n%s/", files[matches[i]]->d_name);
                    }
                    else
                    {
                        printf("\n%s", files[matches[i]]->d_name);
                    }
                }
                printf("\n");

                prompt(last_exit_stat, last_time_taken, 0);
                printf("%s", cmd);

                // printf("\033[u"); /* restores cur position */
                // for (int i = 0; i < additions; i++) {
                //     printf("\033[C");
                // }

                fflush(stdout);
            }

            for (int i = 0; i < nfiles; i++)
            {
                free(files[i]);
            }
            free(files);
        }
        else if (key == 4)
        {
            /* got ctrl D */
            killall_bg();
            printf("\n");
            fflush(stdout);
            exit_gracefully(1);
        }
        else if (key == 10)
        {
            /* input taken */
            printf("\033[J"); /* erases from cursor to end */
            printf("\n");
            fflush(stdout);
            return;
        }
        else
        {
            cmd[p++] = key;
            printf("\033[J"); /* erases from cursor to end */
            printf("%c", key);
            fflush(stdout);
        }
    }
}