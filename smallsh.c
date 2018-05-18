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
 * ** Parameters: size_t buffer_size and a pointer to an array of chars
 *      for getline() to use.
 * ** Pre-Conditions: buffer_size and string ptr must be defined
 * ** Post-Conditions: User input will be stored in string passed to 
 *      function
 * ** Note: Code for this function largely based on Benjamin Brewster's
 *      getline() example on page 3.3 of CS 344 Block 3.
 * *********************************************************************/
void prompt(size_t *size, char *input){
    int char_ct = 0; //For checking getline() success
    char* string_endpt; //For removing whitespace 
    int length; //For storing input length
      
    while(1){
        //Prompt
        printf(": ");
        fflush(stdin);
        
        //Get input
        char_ct = getline(&input, size, stdin);

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
    size_t buffer_size = 0;
    char *user_input = NULL;
    char *args[MAX_ARGS];
    char in_file[MAX_LENGTH], out_file[MAX_LENGTH];
    bool run_in_backgrnd;

    //Run shell
        //Display prompt and get input
        prompt(&buffer_size, user_input);
        //Process input
        //If built-in command, execute
        //Else, if not built-in, find command
            //If valid, fork, handle I/O, execute
            //Else, display error and set exit status to 1
        //Clean up containers
        free(user_input);
        user_input = NULL;

    return 0;
}


