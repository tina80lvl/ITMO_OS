#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
	1) Read:		Read the command from standard input.
	2) Parse:		Separate the command string into a program and arguments.
	3) Execute:		Run the parsed command.
*/
void parse(char *line, char **argv)
{
  while (*line != '\0') {     
    while (*line == ' ' || *line == '\t' || *line == '\n')
      *line++ = '\0';    
    *argv++ = line;        
    while (*line != '\0' && *line != ' ' && 
      *line != '\t' && *line != '\n') 
    line++;            
  }
  *argv = '\0';                 
}

void execute(char **argv)
{
  pid_t  pid;
  int    status;

  if ((pid = fork()) < 0) {    
    printf("*** ERROR: forking child process failed\n");
    exit(1);
  }
  else {
    if (pid == 0) {         
    if (execvp(*argv, argv) < 0) {    
          printf("*** ERROR: exec failed\n");
          exit(1);
        }
      }
      else {                                  
        while (wait(&status) != pid)      
          ;
      }
    } 
}

int main(int argc, char **args) {
	char  line[1024];           
  char  *argv[64];            

  while (1) {               
    printf ("HW-Terminal$ ");
    if (gets(line) == '\0'){
    	printf("\n");
    	exit(0);
    }
    printf("\n");
    parse(line, argv);    
    if (strcmp(argv[0], "exit") == 0) {
      exit(0);        
    }   
    execute(argv);  
    }
}