#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <wordexp.h>
#include "sh.h"
#define BUFFERSIZE 128

void inputToCharArray(char *input,char** cmds)
{
  char* temp;
	temp=strtok(input," ");
	if (temp==NULL)
  {
    cmds[0]=malloc(sizeof(char));
		cmds[0][0]=0;
		return;
	}
  
  int len=strlen(temp);
	cmds[0]=malloc(sizeof(char)*len+1);
	strcpy(cmds[0],temp);
	int i=1;
	while ((temp=strtok(NULL," "))!=NULL)
  {
	  len=strlen(temp);
		cmds[i]=malloc(sizeof(char)*len+1);
		strcpy(cmds[i],temp);
		i++;
	}
		cmds[i]=NULL;
}

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd, *cwd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		          /* Home directory to start out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  cwd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(cwd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();

	signal(SIGINT, handleSigInt);
	signal(SIGTSTP, handleSigStp);
	signal(SIGTERM, handleSigStp);

  while ( go )
  {
    /* print your prompt */
    printf("%s%s$", prompt, pwd);

    /* get command line and process */
    char buffer[BUFFERSIZE];
    fgets (buffer, BUFFERSIZE, stdin);
		int len = strlen(buffer);
		buffer[len-1]=0;

    inputToCharArray(buffer, args);

    /* check for each built in command and implement */
    if (!strcmp(args[0],"exit"))  
    {
      printf("exiting\n");
		  go=0;
		}
		else if (!strcmp(args[0],"which"))   
    {
      printf("executing built-in which\n");
			if (args[1] == NULL)
			{
				printf("which: too few arguments\n");
			}
			else
			{
        for (int i = 1; i < MAXARGS; i++) 
        {
          if (args[i] != NULL)
          {
            char *path = which(args[i], pathlist);
            if (path != NULL) 
            {
              printf("%s\n", path);
              free(path);
            } 
            else 
            {
              printf("%s %s: not found\n", args[0], args[i]);
            }
          }
          else
          {
            break;
          }
        }
			}
		}
		else if (!strcmp(args[0],"where"))
    {
      printf("executing built-in where\n");
      if (args[1] == NULL)
			{
				printf("where: too few arguments\n");
			}
			else
			{
        for (int i = 1; i < MAXARGS; i++) 
        {
          if (args[i] != NULL)
          {
            char *path = where(args[i], pathlist);
            if (path != NULL) 
            {
              printf("%s\n", path);
              free(path);
            } 
            else 
            {
              printf("%s %s: not found\n", args[0], args[i]);
            }
          }
          else
          {
            break;
          }
        }   
      } 
    }
    else if (!strcmp(args[0],"cd"))
    {
      printf("executing built-in cd\n");
      if (args[2])
      {
				fprintf(stderr,"cd: too many arguments\n");
			}
			else if (args[1]) 
      {
				if (!strcmp(args[1],"-"))
        {
					strcpy(pwd,owd);
					free(owd);
					owd = getcwd(NULL,PATH_MAX+1);
					chdir(pwd);
				}
				else 
        {
					free(pwd);
					free(owd);
					owd = getcwd (NULL, PATH_MAX+1);
					pwd = getcwd(NULL, PATH_MAX+1);
          chdir(args[1]);
				}
			}
    }
    else if (!strcmp(args[0],"pwd"))
    {
      printf("executing built-in pwd\n");
      printWD();
    }
    else if(!strcmp(args[0],"list"))
    {
      printf("executing built-in list\n");
      if ((args[1] == NULL) && (args[2] == NULL))
			{
				list(cwd);
			}
			else
			{
				for (int i = 1; i < MAXARGS; i++)
				{
					if (args[i] != NULL)
					{
						printf("[%s]:\n", args[i]);
						list(args[i]);
					}
				}
			}
    }
    else if(!strcmp(args[0],"pid"))
    {
      printf("executing built-in pid\n");
      printPID();
    }
    else if(!strcmp(args[0],"kill"))
    {
      printf("executing built-in kill\n");
      //one argument
      if (args[1] != NULL && args[2] == NULL)
			{
				killPID(atoi(args[1]), 0);
			}
      //two arguments
			else if(args[1] != NULL && args[2] != NULL)
      {
				killPID(atoi(args[2]), -1*atoi(args[1]));
			}
    }
    else if(!strcmp(args[0],"prompt"))
    {
      printf("executing built-in prompt\n");
      newPromptPrefix(args[1],prompt);
    }
    else if(!strcmp(args[0],"printenv"))
    {
      printf("executing built-in printenv\n");
      //zero arguments
      if (args[1] == NULL) 
      { 
        printenv(envp);
      }
      //one argument
      else if((args[1] != NULL) && (args[2] == NULL)) 
      { 
        printf("%s\n", getenv(args[1]));
      }
      //more than one argument
      else 
      {
        perror("printenv");
        printf("printenv: too many arguments\n");
      }
    }
    else if(!strcmp(args[0],"setenv"))
    {
      printf("executing built-in setenv\n");
      //zero arguments
      if(args[1] == NULL)
      {
        printenv(envp);
      }
      //one argument
      else if((args[1] != NULL) && (args[2] == NULL)) 
      { 
        setenv(args[1], "",1);
      }
      //two arguments
      else if((args[1] != NULL) && (args[2] != NULL) && (args[3] == NULL)) 
      {
        setenv(args[1],args[2],1);

        if(!strcmp(args[1], "HOME")) 
        {
					homedir = getenv("HOME");
				}
				if(!strcmp(args[1],"PATH")) 
        {
					free(pathlist);
					pathlist = get_path();
				}
			}
      //more than two arguments
      else 
      { 
				perror("setenv");
				printf("setenv: too many arguments\n");
			}
    }
    /*  else  program to exec */
		else
    {
      int wcIndex = -1;
      int i = 0;
      char **wildcard;
      wordexp_t p;
      while (args[i])
      {
        if (strchr(args[i], '*') || strchr(args[i], '?'))
        {
          wcIndex = i;
        }
        i++;
      }
      if (wcIndex >= 0)
      {
        wordexp(args[wcIndex], &p, 0);
        wildcard = p.we_wordv;
        args[wcIndex] = NULL;
      }

      /* find it */
			//get the absolute path from which
			char* cmd=which(args[0],pathlist);
			pid_t pid=fork();
      /* do fork(), execve() and waitpid() */
			if (pid)
      {
				free(cmd);
				waitpid(pid,NULL,0);
			}
      /* else */
			else
      {
				if (execve(cmd, args, envp) < 0)
				{
					fprintf(stderr, "%s: command not found.\n", args[0]);
					exit(0);
				}
			}
		}
  }
  free(prompt);
	free(commandline);
	free(args);
	free(owd);
	free(pwd);
	free(pathlist->element);
	struct pathelement *tmp = NULL;
	while(pathlist != NULL)
  {
	    tmp = pathlist->next;
	    free(pathlist);
	    pathlist = tmp;
	}
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */

  char* cmd = (char *)malloc(BUFFERSIZE);
  while (pathlist) 
  { //WHICH
    sprintf(cmd, "%s/%s", pathlist->element, command);
    if (access(cmd, X_OK) == 0) 
    {
      return cmd;
    }
    pathlist = pathlist->next;
  }
  free(cmd);
  return NULL;
} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
  char* cmd = (char *)malloc(BUFFERSIZE);
  while (pathlist) 
  { //WHERE
    sprintf(cmd, "%s/%s", pathlist->element, command);
    if (access(cmd, F_OK) == 0) 
    {
      return cmd;
    }
    pathlist = pathlist->next;
  }
  free(cmd);
  return NULL;
} /* where() */

void printWD()
{
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
} /* printWD() */

void list (char *dir)
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
  DIR *dr;
  struct dirent *de;
  dr = opendir(dir);
  if (dr == NULL) 
  {
    perror(dir);
  } 
  else 
  {
    while ((de = readdir(dr)) != NULL) 
    {
      printf("%s\n", de->d_name);
    }
  }
  closedir(dr);
} /* list() */

void printPID()
{
  int pid = getpid();
  printf("shell PID: %d\n", pid);
} /* printPID() */

void killPID(pid_t pid, int sig)
{
	if (sig == 0)
  {
		kill(pid,SIGTERM);
	}
	else 
  {
		kill(pid, sig);
	}
} /* kill() */

void newPromptPrefix(char *command, char *p) 
{
  char buffer[BUFFERSIZE];
  int len;
  if (command == NULL) 
  {
    command = malloc(sizeof(char) * PROMPTMAX);
    printf("Input new prompt prefix: ");
    if (fgets(buffer, BUFFERSIZE, stdin) != NULL) {
    len = (int) strlen(buffer);
    buffer[len - 1] = '\0';
    strcpy(command, buffer);
    }
    strcpy(p, command);
    free(command);
  }
  else 
  {
    strcpy(p, command);
  }
} /* newPromptPrefix() */

void printenv(char **envp)
{
  char **currEnv = envp;
  while (*currEnv)
  {
    printf("%s \n", *(currEnv++));
  }
} /* printenv() */

/* signal handler for Ctrl+C */
void handleSigInt(int sig)
{
	/* Reset handler to catch SIGINT next time.*/
	signal(SIGINT, handleSigInt);
	printf("\n cannot be terminated using Ctrl+C %d \n", waitpid(getpid(), NULL, 0));
	fflush(stdout);
	return;
}

/* signal handler for Ctrl+Z */
void handleSigStp(int sig)
{
	signal(SIGTSTP, handleSigStp);
	printf("\n cannot be terminated using Ctrl+Z \n");
	fflush(stdout);
}
