/*********************************************************************
 * ** Program Filename: smallsh.c
 * ** Author: Kendra Ellis <ellisken@oregonstate.edu>
 * ** Date: May 15, 2018
 * ** Description: Simple shell program written for CS 344
 * **   Spring 2018.
 * *********************************************************************/
/* Other tasks:
 * Parse input of ‘$$’ as current process id
 * Wait for completion of foreground commands before prompting again
 * Do not wait for background commands to complete, return command line access
 * and control to user immediately AFTER forking. Periodically check for
 * background child processes to complete with waidpid(...NOHANG…))
 * Store PIDs of non-completed background processes in an array?
 * Print out background process completed BEFORE command line access and control
 * are returned to the user
 * Redirect background stdin from /dev/null, redirect stdout to /dev/null if no
 * other target
 * Print process id of background process when the process begins
 * When terminates, show message with PID and exit status. 
 * Check bckgrnd processes complete BEFORE prompting for a new command
 * Change behavior of CTRL-C (SIGINT) and CTRL-Z to (SIGTSTP)
 *
 * */
/* List of functions:
 * getInput()
 * processInput()
 * handleIO()
 * exit()
 * cd()
 * status()
 * Prompt()
 * execOther()
 * execBuiltIn()
 * error()
 * cleanup()
 * */

#include <stdbool.h> //For bools
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#define MAX_LENGTH 2048
#define MAX_ARGS 512

void catchSIGINT(int signo){
    char* message = "SIGINT. Use CTRL-Z to Stop.\n";
    write(STDOUT_FILENO, message, 28);
}




/************************************************************************
 * ** Function: prompt()
 * ** Description: Displays the shell prompt ":" and receives user input;
 *      Strips user input of leading and trailing whitespace.
 * ** Parameters: None
 * ** Note: Code for this function largely based on Benjamin Brewster's
 *      getline() example on page 3.3 of CS 344 Block 3.
 * *********************************************************************/
void prompt(char *line){
    size_t buffer_size = 0;
    char *input = NULL;
    int char_ct = 0; //For checking getline() success
    char* string_endpt; //For removing whitespace 
   
    while(1){
        //Prompt
        printf(": ");
        fflush(stdout);
        
        //Get input
        char_ct = getline(&input, &buffer_size, stdin);

        //Check for getline() interruption error and clear error status
        if(char_ct == -1){
            clearerr(stdin);
            char_ct = 0;
        }
        else break;
    }

    //Find start of string (skip leading whitespace)
    string_endpt = &input[strspn(input, " \t\n")];
    //Reassign input to new starting point
    input = string_endpt;
    //Get string length and find actual endpoint,
    //strip trailing whitespace
    string_endpt = input + strlen(input);
    while(isspace(*--string_endpt))
        *(string_endpt + 1) = '\0';
    //Take care of last whitespace char
    *(string_endpt + 1) = '\0';
    //Do not copy if error or blank line
    if(input == NULL || strlen(input) == 0) return;
    else{ 
        strcpy(line, input);
        free(input);
    }
    return;
}


/************************************************************************
 * ** Function: process_input()
 * ** Description: Parses user input into command, args, and background 
 *      "&" option. Returns true if successful, returns false if input
 *      is a comment line (starts with "#") or blank. Does not check
 *      for correct user input syntax (space delimited).
 * ** Parameters: User input string, array of pointers to store args,
 *      strings to hold names of infile & outfile, bool for background
 *      process option.
 * *********************************************************************/
bool process_input(char *line, char *command, char *args[], char *in, char *out, bool *backgrnd){
    char *item = NULL; //For tokenizing the input line
    int arg_ct = 0;
    //If string does not exist, is empty, or is a comment return false
    if(line == NULL || strlen(line) == 0 || line[0] == '#') return false;

    //Else, tokenize string by spaces
    //First token is saved in "command"
    item = strtok(line, " ");
    strcpy(command, item);
    printf("Command: %s\n", command);
    item = strtok(NULL, " ");
    while(item != NULL){
        switch(*item){
            //If token is "<" save next word in infile
            case '<':
                item = strtok(NULL, " ");
                strcpy(in, item);
                break;
            //If token is ">" save next word in outfile
            case '>':
                item = strtok(NULL, " ");
                strcpy(out, item);
                break;
            //If token is "&", change value of run_in_background
            case '&':
                *backgrnd = true;
                break;
            //Else, save in args[]
            default:
                args[arg_ct++] = item;
                printf("arg added: %s\n", args[arg_ct - 1]);
        }
        item = strtok(NULL, " ");
    }
    return;
    
}


/***********************************************************************
 *                                 MAIN
 * ********************************************************************/
int main(){
    //Define signal handling
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = catchSIGINT;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL);
    
    //Containers for input, command, args, and files
    char user_input[MAX_LENGTH];
    char command[MAX_LENGTH];
    char *args[MAX_ARGS];
    char in_file[MAX_LENGTH], out_file[MAX_LENGTH];
    bool run_in_backgrnd;
    bool valid;

    //Run shell
        //Display prompt and get input
        memset(user_input, '\0', sizeof(user_input));
        prompt(user_input);
        fflush(stdout);
        //Process input
        memset(command, '\0', sizeof(command));
        memset(in_file, '\0', sizeof(in_file));
        memset(out_file, '\0', sizeof(out_file));
        valid = process_input(user_input, command, args, in_file, out_file, &run_in_backgrnd);
        //If built-in command, execute
        //Else, if not built-in, find command
            //If valid, fork, handle I/O, execute
            //Else, display error and set exit status to 1
        //Clean up containers
        //user_input = NULL;
    return 0;
}


