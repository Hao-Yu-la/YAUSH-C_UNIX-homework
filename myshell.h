#ifndef _MYSHELL_H
#define _MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

void print_prompt();
void partition_cmd(char *buf, int *argcount, char (*arglist)[]);
void do_pipe_cmd(int argcount, char (*arglist)[]);
void do_simple_cmd(int argcount, char *arg[], int *prefd, int *postfd, int background);
int find_cmd(char *command);


// print prompt
void print_prompt()
{
	printf("[zhanghy的shell]:");
    return;
}

// partition command, get arguments, and save them in arglist, partition by space
void partition_cmd(char *buf, int *argcount, char arglist[100][256])
{
    char *p = buf;
    char *q = buf;
    int number = 0;

    while(1)
    {
        if(p[0] == '\0')
            break;
        if(p[0] == ' ')
            p++;
        else
        {
            q = p;
            number = 0;
            while((q[0]!=' ') && (q[0]!='\n') && (q[0]!='\0'))
            {
                number++;
                q++;
            }
            strncpy(arglist[*argcount], p, number+1);
            arglist[*argcount][number] = '\0';
            *argcount = *argcount + 1;
            p = q;
        } 
    }

    return;
}

void do_pipe_cmd(int argcount, char arglist[100][256])
{
    int i;
    int background = 0;
	char* arg[argcount+1];

	for (i = 0; i < argcount; i++)
	{
		arg[i] = (char *)arglist[i];
	}
	arg[argcount] = NULL;

	// if command ends with "&", then it is a background command
    for (i = 0; i < argcount; i++)
	{
		if(strncmp(arg[i], "&", 1) == 0)
		{
			if(i == argcount-1)
			{
				background = 1;
				arg[argcount - 1] = NULL;
				break;
			}
			else
			{
				printf("wrong command, because '&'\n");
				return;
			}
		}
	}

    // if command contains "|", then it is a pipe command, create pipe and divide it into two commands
    int j = 0;
    int prepipe = 0;  
    int prefd[2], postfd[2]; // 前管道，与前面命令间的管道；后管道，与后面命令间的管道
    int pid[10] = {0};
    int stream_id = 0;
    int status;

    for (i = 0; arg[i] != NULL; i++)
    {
        if (strncmp(arg[i], "|", 1) == 0)
        {
            arg[i] = NULL;
            pipe(postfd); // create pipe

            if ((pid[stream_id] = fork()) < 0)
            {
                printf("fork error\n");
                return;
            }
            else if (pid[stream_id] == 0)
            {
                if (prepipe)
                {
                    do_simple_cmd(i-j, arg+j, prefd, postfd, background);
                }
                else
                {
                    do_simple_cmd(i-j, arg+j, NULL, postfd, background);
                }    
            }
            prepipe = 1;
            prefd[0] = postfd[0];
            prefd[1] = postfd[1];
            j = i + 1;
            stream_id++;
        }
    }

    // process the last command
    if ((pid[stream_id] = fork()) < 0)
    {
        printf("fork error\n");
        return;
    }
    else if (pid[stream_id] == 0)
    {
        if (prepipe)
        {
            do_simple_cmd(i-j, arg+j, prefd, NULL, background);
        }
        else
        {
            do_simple_cmd(i-j, arg+j, NULL, NULL, background);
        }
    }
    stream_id++;

    if (background == 0)
    {
        // for (i = 0; i < stream_id; i++){
        //     waitpid(pid[i], &status, 0);
        // }
        for (i = 0; i < stream_id; i++){
            printf("pid[%d] = %d\n", i, pid[i]);
        }
        waitpid(pid[0], &status, 0);
    }
    else
    {
        // for (i = 0; i < stream_id; i++){
        //     waitpid(pid[i], &status, WNOHANG);
        // }
        waitpid(pid[0], &status, WNOHANG);
    }

    return;  
}

void do_simple_cmd(int argcount, char *arg[], int *prefd, int *postfd, int background)
{
    int i;
    // int status;
    // int pid;
    int fd;

    
    if (prefd != NULL)
    {
        close(prefd[1]);
        dup2(prefd[0], STDIN_FILENO);
        close(prefd[0]);
    }
    if (postfd != NULL)
    {
        close(postfd[0]);
        dup2(postfd[1], STDOUT_FILENO);
        close(postfd[1]);
    }

    // if command contains ">", ">>" or "<", then it is redirection command
    for ( i = 0; i < argcount; i++)
    {
        if (strncmp(arg[i], ">", 1) == 0)
        {
            if (postfd != NULL)
            {
                printf("wrong command, because '>'\n");
                return;
            }
            
            fd = open(arg[i+1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            arg[i] = NULL;
        }
        else if (strncmp(arg[i], ">>", 2) == 0)
        {
            if (postfd != NULL)
            {
                printf("wrong command, because '>>'\n");
                return;
            }
            
            fd = open(arg[i+1], O_WRONLY|O_CREAT|O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            arg[i] = NULL;
        }
        else if (strncmp(arg[i], "<", 1) == 0)
        {
            if (prefd != NULL)
            {
                printf("wrong command, because '<'\n");
                return;
            }

            fd = open(arg[i+1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            arg[i] = NULL;
        }
    }

    // execute command
    if(!(find_cmd(arg[0])))
    {
        printf("%s : command not found\n",arg[0]);
        exit(0);
    }

    if (execvp(arg[0], arg) < 0)
    {
        printf("cannot execute command %s\n", arg[0]);
        exit(0);
    }


    return;
}

int find_cmd(char *command)
{
    DIR *dp;
    struct dirent *dirp;
    char *path[] = {"./", "/bin", "/usr/bin", NULL};

    // look for the command in the current directory and in the path
    if(strncmp(command, "./",2) == 0)
    {
        command = command + 2;
    }

    int i = 0;
    while (path[i] != NULL)
    {
        if ((dp = opendir(path[i])) == NULL)
        {
            printf("can't open %s\n", path[i]);
        }
        while ((dirp = readdir(dp)) != NULL)
        {
            if (strcmp(dirp->d_name, command) == 0)
            {
                closedir(dp);
                return 1;
            }
        }
        closedir(dp);
        i++;
    }

    return 0;
}



#endif