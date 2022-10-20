# mish
A minimal shell written in C

## Installation
run `make` in the shell directory.
then run `./sh`.

## Features
* cd - change directories to navigate the file system
* pwd - print the current working directory
* echo - print the arguments passed to it
* ls - list all files and sub directories inside a directory.
* discover - minimal find
* history - stores 20 of the previous commands
* pinfo - prints basic process info
* all system commands
* foreground and background processes
* job control (jobs, fg, bg, sig)
* autocompletion for folders and files
* input and output redirection
* pipes ('|') between commands

## Work
### sh.c
Contains the main logic of the program, it also has an `init()` function which sets up various shell related conditions.

### core.c
Contains cd, pwd, echo and ls, are considered to be at the core of the shell

### history.c, pinfo.c, discover.c
contains the code of the command they are named after

### utils.c
contains routines to various convenience functions like `trim()` etc.
also contains the main parser and task execution.
also contains functions to redirect input and piping.

### list.c
contains a singly linked list implementation for message queues and process queues.
also has field such as stopped and terminated for job control.

### prompt.c
displays the prompt that asks the user for input

### jobs.c, fg.c, bg.c, sig.c
implements the respective command

## Assumptions
* LS_BLOCK_RATIO: 2 - the size of an ls block to the size of a system block (512B)
* CMD_MAX: 4096 - the maximum cmd length
* PATH_MAX: 4096 - maximum path length (system dependent)
* HOSTNAME_MAX: 256 - max hostname length (system dependent)
* TIME_MAX: 4096 - any time is atmost 4096 bytes long
* BG_MAX: 512 - the maximum number of background processes
* PROMPT_MAX: PATH_MAX + HOSTNAME_MAX - sum of path length (to display PATH_MAX) and hostname (to display HOSTNAME_MAX).
* Termination messages when a background process ends are queued and are outputted before the shell asks for its next prompt. The order of these messages do not matter.
* The number of files to be searched for in discover will always be one.
* When running a background process, the number in [ <number here> ] square brackets is the number of currently running background processes.
* Ctrl-C exits only terminates the current process and not all the process in a process chain. This is true for commands in a pipeline as well.
* only valid input will be given, anything invalid will lead to undefined behaviour when using pipes or redirection.
* always use the terminal in full screen, the behavious of autocompletion is undefined when the number of files printed is greater than terminal height.
