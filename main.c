#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h> 

int main(int argc, char *argv[])
{
    char* line;
    // read_command(line);  
    while(1)
    { 
        line = readline(">> ");
        if (line == NULL)
        {
            printf("END\n");
            break;
        }
        else {
            printf("%s\n", line); 
        }
    }
    return 0;
}