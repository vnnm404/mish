#ifndef _HISTORY_H_
#define _HISTORY_H_

/*
    all history is saved inside a file called
    .sh_history in the same directory
    as the sh executable
*/

extern char history_file_name[];
extern FILE *histfp;
extern off_t history_newest, history_oldest;

int add_to_history();
int sh_history(int nargs, char *args[]);

#endif /* _HISTORY_H_ */