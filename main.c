#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h> 
#include "myshell.h"

int main(int argc, char *argv[])
{
    char* line = NULL;
    int argcount = 0;
	char arglist[100][256];
    
    while(1)
    { 
        //init
        line = NULL;
        argcount = 0;
        memset(arglist, 0, sizeof(arglist));
        
        // Read a command line from stdin
        print_prompt();
        line = readline(">> ");
        if (strcmp(line, "exit\0\n") == 0)
        {
            printf("END\n");
            break;
        }
        // process command line
        else {
            printf("%s\n", line);
            partition_cmd(line, &argcount, arglist); 
            printf("argcount = %d\n", argcount);
            do_pipe_cmd(argcount, arglist);
        }
    }
    return 0;
}