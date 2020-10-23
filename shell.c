/*

    Name: Benjamin Knight 
    ID: 1001788622

    CSE-3320-002 Shell Project
    Goal: 
    To create a shell identical to the one we use in Fedora that will 
    support commands such as "ls" or "cd" and 10 arguments with it as 
    well. Any code professor provides us we are allowed to use. In 
    particular we will be using the string parsing code he provides
    so we do not have to hassle with that.

*/

//abc def ghi jkl mno pqrs tuv wxyz ABC DEF GHI JKL MNO PQRS TUV WXYZ !"§ $%& /() =?* '<> #|; ²³~ @`
// ^ above is 100 chars of text (including slashes) to keep inline with professors req 15

#define GNU_SOURCE

#include <stdio.h>  
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>


/*
  We'll be splitting up the command line input
  into tokens, these chars (space, \t, \n) are 
  are what delimits our tokens
*/
#define WHITESPACE " \t\n"

// The maximum command-line size
#define MAX_COMMAND_SIZE 255   

// The num of builtin functions
#define NUM_OF_BUILTINS 5

// shell only supports 10 arguments in ADDITION to command so 11
#define MAX_NUM_ARGUMENTS 11  

// keep track of commands entered, global to avoid passing through functions
int hist_items = 0; 
int pid_items = 0;


/* 
  FUNCTIONS: 

  check_if_builtin():
  Will check if the input  is a "builtin" function
  if it is, the commanded function will run and the
  main loop will return without any further execution
  else returns -1.

  run_command():
  Command entered was deemed not a "builtin" funct
  and will attempt to execute it.

  run_builtin():
  Will run one of the "builtin" functions such as:
  cd, history, showpids, quit, exit.

  show_hist():
  Prints history to console.

  add_to_hist():
  Adds command line input to history table.

  change_directory():
  Changes the current working directory to the one 
  specified.

  check_if_empty():
  Verifies if the entered string is nothing but delimiters.

  add_pid():
  Adds any created process's ID numbers to the table
  available for display.

  show_pids():
  Displays any pids from created processes
*/
int check_if_builtin(char * token[]);

pid_t run_command(char * token[]);

int run_builtin(char * token[], char * built_ins[], char * hist[], pid_t p_tbl[]);

void show_hist(char * hist[]);

void add_to_hist(char str[], char * hist[]);

void change_directory(char * token[]);

int check_if_empty(char str[]);

void add_pid(pid_t pid, pid_t p_tbl[]);

void show_pids(pid_t p_tbl[]);

int main()
{
  int i;

  // Will hold all of our pids for show_pids();
  pid_t * p_tbl = (pid_t*)malloc(15 * sizeof(pid_t));

  // This is the string we will be storing command line input into
  char * cmd_str = (char*) malloc(MAX_COMMAND_SIZE);

  // Holds all possible builtin functions user may request
  char ** built_ins = malloc(NUM_OF_BUILTINS * sizeof(char *));
  built_ins[0] = (char*)malloc(2); // "cd"
  built_ins[1] = (char*)malloc(7); // "history"
  built_ins[2] = (char*)malloc(8); // "showpids"
  built_ins[3] = (char*)malloc(4); // "quit"
  built_ins[4] = (char*)malloc(4); // "exit"
  built_ins[0] = "cd";
  built_ins[1] = "history";
  built_ins[2] = "showpids";
  built_ins[3] = "quit";
  built_ins[4] = "exit";

  /*
    Initalize history table, we don't malloc every
    array item here, because we will malloc a "tmp"
    location for it individually
  */
  char ** hist = malloc(15 * sizeof(char *));

  while(1)
  {
    printf("msh> "); 

    /*
      waits until text is typed into the command line
      (this is why an infinite loop of "shell>" doesnt happen)
      stores it into cmd_str, and if nothings input returns null
    */
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    // Will check if the entered string is nothing but delimiters
    if(check_if_empty(cmd_str) == -1)
    {
      continue;
    }

    // We create tmp out here in main so we can free it
    char * tmp = malloc(MAX_COMMAND_SIZE);
    strcpy(tmp, cmd_str);
    add_to_hist(tmp, hist);

    char * token[MAX_NUM_ARGUMENTS]; // Storing cmds/args

    int token_count = 0;

    char * arg_ptr; // Points to current token

    char * working_str = strdup(cmd_str);

    char * working_root = working_str; // Keep track of origin to free

    /*
      This is our loop that is actually tokenizing the input.
      The values inside of token[] are char*, this means 
      that each one points towards an entire char[] (string)
      even though we haven't labeled it as such because we are
      doing it dynamicall through pointers 
    */
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
            (token_count<MAX_NUM_ARGUMENTS))
    {
        token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
        if( strlen( token[token_count] ) == 0 )
        {
            token[token_count] = NULL;
        }
        token_count++;
    }

    free( working_root ); // Avoid mem leaks

    if(check_if_builtin(token) > 0)
    {
      if(run_builtin(token, built_ins, hist, p_tbl) < 0) // "exit or quit called"
      {
        // avoids mem leaks
        free(built_ins);
        free(cmd_str);
        free(tmp);
        exit(0);
      }
    }
    else
    { 
      add_pid(run_command(token), p_tbl); // Run entered command and add its pid
    }
  } 
  // END OF MAIN LOOP

  // free before exiting as insurance 
  free(built_ins);
  free(cmd_str);
}
// END OF MAIN


int check_if_builtin(char * token[])
{
  int ret = -1;

  // These are all the builtins professor requests us to keep include
  if((strcmp(token[0],"quit") == 0) || (strcmp(token[0],"exit") == 0) 
  || (strcmp(token[0],"cd") == 0) || (strcmp(token[0],"history") == 0)
  || (strcmp(token[0],"showpids") == 0))
  {
    ret = 1;
  }

  // For the !n command
  char * str = token[0];
  char x = str[0];
  if(x == '!')
  {
    ret = 1;
  }
  
  return ret; // 1 = it's a builtin
}

pid_t run_command(char * token[])
{
  int i;
  int ret;

  pid_t pid = fork();

  if(pid == 0)
  {
    if(execvp(token[0], token) == -1)
    {
      printf("%s: Command not found.\n", token[0]);
    }
    exit(0);
  }
  else if(pid > 0)
  {
      wait(NULL); // Wait for child to die
  }

  return pid;
}

int run_builtin(char * token[], char * built_ins[], char * hist[], pid_t p_tbl[])
{
  int i, tmp, ret = 1;
  
  char * str = token[0];

  // Loop finds which builtin function was called 
  for(int i = 0; i < 5 ; i++)
  {
    if(strcmp(str, built_ins[i]) == 0)
    {
      tmp = i;
      break;
    }
  }
  
  // Handles !n command
  if(str[0] == '!')
  {
    // i was 5 because it finished the loop
    tmp = 6;
  }

  switch(tmp)
  {
    // "cd" called
    case 0:
    {
      change_directory(token);
      break;
    }

    // "history" called
    case 1:
    {
      show_hist(hist);
      break;
    }

    // "showpids" called
    case 2:
    {
      show_pids(p_tbl);
      break;
    }

    // "quit" called
    case 3:
    {
      ret = -1; // Signals to main we're exiting
      break;
    }

    // "exit" called
    case 4:
    {
      ret = -1; // Signals to main we're exiting
      break;
    }

    // "!n called"
    case 6:
    {
      // Clean up string
      str[0] = str[1];
      str[1] = str[2];
      str[2] = '\0';

      /*
        We know that there is no more than 15 hist
        entries so we dont have to worry about any number
        in the hundredths which is why we can hardcode this 
      */

      int n = atoi(str);
      printf("Command !n has not been implemented, due to structure of code.\n");

      break;
    }

    default:
      break;
  }

  return ret;
}

void show_hist(char * hist[])
{
  int i;
  for(i = 0; i < hist_items; i++)
  {
    printf("%d: %s\n", i, hist[i]);
  }
}

void add_to_hist(char str[], char * hist[])
{
  /*
    We store in the input string into a dynamically allocated
    tmp string so we can just pass the char * towards out hist
    array. Also clean off the newline character of the input
    for cleanliness when outputting.
  */
  int i, len = strlen(str);
  str[len-1] = '\0';

  // Before history table is filled up, simply adds commands into it
  if(hist_items < 15)
  {
    hist[hist_items] = str;
    hist_items++;
  }
  else
  {
    // So we can free this data in hist table;
    char * tmp;
    tmp = hist[0];

    // Moving everything in the table up by one
    for(i = 0; i < 14; i++)
    {
      hist[i] = hist[i+1];
    }

    // Our new hist input
    hist[14] = str;

    // Freeing the old input
    free(tmp);
  }
}
 
void change_directory(char * token[])
{
  // The chdir() function will change the running process's directory
  if(chdir(token[1]) < 0)
  {
    printf("ERROR: No directory labeled %s\n", token[1]);
  }
}

int check_if_empty(char str[])
{
  // Return assumes str is empty
  int ret = -1;

  int i, len = strlen(str);

  int curr_char;

  for(i = 0; i < len; i++)
  {
    curr_char = str[i];

    // If curr char does not equal any of these " \n\t", str not empty
    if(curr_char != ' ' && curr_char != '\n' && curr_char != '\t')
    {
      ret = 1;
    }
  }
  return ret;
}

void add_pid(pid_t pid, pid_t * p_tbl)
{
  int i;

  // While we have created <15 pids
  if(pid_items < 15)
  {
    p_tbl[pid_items] = pid;
    pid_items++;
  }
  // After creating >15 pids
  else
  {
    for(i = 0; i < pid_items - 2; i++)
    {
      p_tbl[i] = p_tbl[i+1];
    }
    p_tbl[pid_items-1] = pid;
  }
}

void show_pids(pid_t p_tbl[])
{
  int i;
  for(i = 0; i < pid_items; i++)
  {
    printf("%d: %d\n", i, p_tbl[i]);
  }
}
