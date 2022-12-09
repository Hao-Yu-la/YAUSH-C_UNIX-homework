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
    char line_history[20][512];
    memset(line_history, 0, sizeof(line_history));
    int line_history_count = 0;
    
    while(1)
    { 
        //init
        line = NULL;
        argcount = 0;
        memset(arglist, 0, sizeof(arglist));

        // Read a command line from stdin
        print_prompt();
        line = readline(">> ");
        memset(line_history[line_history_count%20], 0, sizeof(line_history[line_history_count%20]));
        memcpy(line_history[line_history_count%20], line, strlen(line));
        line_history_count++;
        
        if (strcmp(line, "exit\0\n") == 0)
        {
            printf("END\n");
            break;
        }

        if (strcmp(line, "history\0\n") == 0)
        {
            int command_num = line_history_count > 20 ? 20 : line_history_count;
            for (int i = 0; i < command_num; i++)
            {
                int index = (line_history_count - command_num + i) % 20;
                printf("%s\n", line_history[index]);
            }
        }

        // process command line
        else {
            printf("%s\n", line);
            partition_cmd(line, &argcount, arglist); 
            // printf("argcount = %d\n", argcount);
            do_pipe_cmd(argcount, arglist);
        }
    }
    return 0;
}