#ifndef _DISCOVER_H_
#define _DISCOVER_H_

int sh_discover(int nargs, char *args[]);
void sh_discover_r(char *path, char *search_for);

extern char files_only;
extern char dirs_only;

#endif /* _DISCOVER_H_ */