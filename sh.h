
#include "get_path.h"

int pid;
void inputToCharArray(char *input,char** cmds);
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void list (char *dir);
void printWD();
void printPID();
void killPID(pid_t pid, int sig);
void newPromptPrefix(char *cmd, char *prompt);
void printenv(char **envp);
void printenv_arg(char **arg);
void handleSigInt(int sig);
void handleSigStp(int sig);

#define PROMPTMAX 32
#define MAXARGS 10
