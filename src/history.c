#include "../include/global.h"
#include "../include/history.h"

char history_file_name[PATH_MAX];
FILE *histfp;
off_t history_newest, history_oldest;

int add_to_history() {
    int lines = 0;
    char buf[CMD_MAX];
    char buff[20][CMD_MAX];

    fseek(histfp, 0, SEEK_SET);
    fgets(buf, CMD_MAX, histfp);
    while (!feof(histfp)) {
        strcpy(buff[lines], buf);
        lines++;

        fgets(buf, CMD_MAX, histfp);
    }

    strcat(cmd_copy, "\n");
    if (strcmp(buff[lines - 1], cmd_copy) == 0)
        return TSK_OK;

    if (lines == 20) {
        for (int i = 1; i < 20; i++) {
            strcpy(buff[i - 1], buff[i]);
        }
        strcpy(buff[19], cmd_copy);
    } else {
        strcpy(buff[lines], cmd_copy);
        lines++;
    }

    histfp = freopen(NULL, "w+", histfp);
    for (int i = 0; i < lines; i++)
        fprintf(histfp, "%s", buff[i]);

    fflush(histfp);

    return TSK_OK;
}

int sh_history(int nargs, char *args[]) {
    if (nargs > 1) {
        fprintf(stderr, "history: invalid args\n");
        return TSK_FAIL;
    }

    int lines = 0;
    char buf[CMD_MAX];
    char buff[20][CMD_MAX];

    fseek(histfp, 0, SEEK_SET);
    fgets(buf, CMD_MAX, histfp);
    while (!feof(histfp)) {
        strcpy(buff[lines], buf);
        lines++;

        fgets(buf, CMD_MAX, histfp);
    }

    if (lines <= 10) {
        for (int i = 0; i < lines; i++)
            printf("%s", buff[i]);
    } else {
        int s = lines - 10;
        for (int i = s; i < lines; i++)
            printf("%s", buff[i]);
    }

    return TSK_OK;
}