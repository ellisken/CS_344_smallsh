/*********************************************************************
 * ** Program Filename: smallsh.c
 * ** Author: Kendra Ellis <ellisken@oregonstate.edu>
 * ** Date: May 15, 2018
 * ** Description: Simple shell program written for CS 344
 * **   Spring 2018.
 * *********************************************************************/
/* Other tasks:
 * ****Parse input of ‘$$’ as current process id****
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#define MAX_LENGTH 2048
#define MAX_ARGS 512

//Global variable for controlling foreground-only mode,
//Changed by the SIGTSTP handling function
bool no_backgrnd = false; //Default value is false

/************************************************************************
 * ** Function: prompt()
 * ** Description: Displays the shell prompt ":" and receives user input;
 *      Returns true if valid input, otherwise returns false if comment
 *      or blank line.
 * ** Parameters: None
 * *********************************************************************/
bool prompt(char *line){
    //Prompt
    printf(": ");
    fflush(stdout);

    //Get input
    line[0] = '\0';
    fgets(line, MAX_LENGTH, stdin);
        
    //If comment or blank line, return false
    if(line[0] == '#' || line[0] == '\n' || line[0] == '\0'){
        return false;
    }

    else return true;
}


/************************************************************************
 * ** Function: process_input()
 * ** Description: Parses user input into args (where arg[0] is
 *      the user-entered command), and background "&" option. 
 * ** Parameters: User input string, array of pointers to store args,
 *      strings to hold names of infile & outfile, bool for background
 *      process option, pointer to char to store parent PID.
 * *********************************************************************/
void process_input(char *line, char *pid, char *args[], char *in, char *out, bool *backgrnd){
    char *item = NULL; //For tokenizing the input line
    int arg_ct = 0;

    //Else, tokenize string by spaces
    //First token is saved in "command"
    //item = strtok(line, " \n");
    //strcpy(command, item);
    item = strtok(line, " \n");
    while(item != NULL){
        switch(*item){
            //If token is "<" save next word in infile
            case '<':
                item = strtok(NULL, " \n");
                strcpy(in, item);
                break;
            //If token is ">" save next word in outfile
            case '>':
                item = strtok(NULL, " \n");
                strcpy(out, item);
                break;
            //If token is "&", change value of run_in_background
            case '&':
                *backgrnd = true;
                break;
            //Else, save in args[]
            default:
                //Expand process ID if needed
                if(strcmp(item, "$$") == 0){
                    //memset(pid, '\0', sizeof(pid));
                    snprintf(pid, 10, "%d", (int)getpid());
                    args[arg_ct++] = pid;
                    printf("$$ expanded to: %s\n", pid);
                    fflush(stdout);
                }
                else{
                    args[arg_ct++] = item;
                    fflush(stdout);
                }
        }
        item = strtok(NULL, " \n");
    }
    return;
}



/************************************************************************
 * ** Function: change_dir()
 * ** Description: With no parameter specified, will change to the
 *      directory specified in the HOME environment variable. Otherwise,
 *      will change to the directory specified by the user-entered path.
 * ** Parameters: Takes the directory name, which will either be NULL or
 *      a directory path
 * *********************************************************************/
void change_dir(char **args){   
    //If directory specified, change to that directory
    if(args[1] != NULL){
        chdir(args[1]);
    }
    //Else, change to the HOME directory
    else chdir(getenv("HOME"));
    return;
}



/************************************************************************
 * ** Function: status()
 * ** Description: Prints either the exit status or the terminating
 *      signal of the last foreground process run by the shell. From 
 *      Benjamin Brewster's lecture.
 * ** Parameters: int for retrieving the exit or termnation status.
 * *********************************************************************/
void status(int exit_status){    
    //If exit status, print exit value
    if(WIFEXITED(exit_status)){
        printf("exit value %d\n", WEXITSTATUS(exit_status));
        fflush(stdout);
    }
    //Else, print terminating signal of last process
    else{
        printf("terminated by signal %d\n", WTERMSIG(exit_status));
        fflush(stdout);
    }
    return;
}



/************************************************************************
 * ** Function: check_background()
 * ** Description: Checks for the background child processes that have
 *      finished. If a process has terminated, displays a message
 *      showing the process id and exit status.
 * ** Parameters: int for retrieving the exit or termnation status.
 * *********************************************************************/
void check_background(){
    pid_t cpid; //For storing the terminated process's id
    int signal; //For storing exit status
    
    //Wait for any child process, return immediately if none have exited
    cpid = waitpid(-1, &signal, WNOHANG);
    while(cpid > 0){
        //Show PID of exited process
        printf("background pid %d is done: ", cpid);
        fflush(stdout);
        //If exited, show exit status,
        //Else, show terminating signal
        status(signal);
        cpid = waitpid(-1, &signal, WNOHANG);
    }
    return;
}



/************************************************************************
 * ** Function: catchSIGTSTP()
 * ** Description: Catchable by the shell only. Turns foreground-only
 *      mode on or off depending on value of global variable 
 *      "no_backgrnd"
 * *********************************************************************/
void catchSIGTSTP(int signo){
    char *foreground_on = "\nEntering foreground-only mode (& is now ignored)\n";
    char *foreground_off = "\nExiting foreground-only mode\n";

    //If no_backgrnd is false, turn foreground-only mode on
    if(no_backgrnd == false){
        write(STDOUT_FILENO, foreground_on, 50);
        fflush(stdout);
        no_backgrnd = true;
    }
    //Else, turn foreground-only mode off
    else{
        write(STDOUT_FILENO, foreground_off, 30);
        fflush(stdout);
        no_backgrnd = false;
    }
}



/***********************************************************************
 *                                 MAIN
 * ********************************************************************/
int main(){
    //Define signal handling
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;//Set up to ignore signals
    sigfillset(&SIGINT_action.sa_mask);//Block/delay all signals arriving
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    //Define struct for handling SIGTSTP
    SIGTSTP_action.sa_handler = catchSIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGTSTP);
    
    //Containers for input, command, args, files, etc.
    char *builtin_commands[3] = {"exit", "status", "cd"};
    char *user_input = malloc(sizeof(char) * MAX_LENGTH);
    char *pid = malloc(sizeof(char) * 10); //For converting the PID into a string
    char *args[MAX_ARGS];
    char in_file[MAX_LENGTH], out_file[MAX_LENGTH];//For I/O file names
    int infile = -1;
    int outfile = -1;
    int devnull = -1;//For redirecting background I/O to /dev/null
    int exit_status;
    bool exit_shell = false;//Control prompt loop
    bool run_in_backgrnd; //Indicates background/foreground process
    bool valid = false; //For input validation
    int result; //For use with dup2()
    int i;
    pid_t cpid, exit_pid;//For storing process IDs

    //Run shell
    while(!exit_shell){


        //Display prompt and get valid input
        while(!valid){
            valid = prompt(user_input);
        }

        //Prepare containers for input   
        memset(in_file, '\0', sizeof(in_file));
        memset(out_file, '\0', sizeof(out_file));
        //Set each arg pointer to NULL
        for(i = 0; i < MAX_ARGS; i++){
            args[i] = NULL;
        }
        //Process input
        process_input(user_input, pid, args, in_file, out_file, &run_in_backgrnd);
        
        //If built-in command to run in foreground, execute 
        if(strcmp(args[0], "status") == 0){      
            status(exit_status);
        }
        else if(strcmp(args[0], "cd") == 0){
            change_dir(args);
        }
        else if(strcmp(args[0], "exit") == 0){
            exit_shell = true;
        }
    
        
        //Else, if not built-in, fork, handle I/O, find command
        else{
            //Create fork
            cpid = fork();
            switch(cpid){
                //If fork successful:
                case 0:
                    //Ignore SIGTSTP in child process
                    SIGTSTP_action.sa_handler = SIG_IGN;
                    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

                    //If foreground process or foreground-only mode,
                    //set to default SIGINT handling
                    if(!run_in_backgrnd || no_backgrnd){
                        SIGINT_action.sa_handler = SIG_DFL;
                        sigaction(SIGINT, &SIGINT_action, NULL);
                    }
                    
                    //Handle input file
                    if(strcmp(in_file, "") != 0 && in_file != NULL){
                        //Open input file
                        infile = open(in_file, O_RDONLY);
                        //Check opened correctly
                        if(infile == -1){
                            printf("cannot open %s for input\n", in_file);
                            fflush(stdout);
                            exit(1);
                        }
                        //In-file redirection
                        result =  dup2(infile, 0);
                        if(result == -1){
                            perror("dup2 in\n");
                            exit(2);
                        }
                        close(infile);
                    }
                    //If background process, foreground-only mode
                    //is disabled, and input file not specified, 
                    //redirect to /dev/null
                    else if(run_in_backgrnd && !no_backgrnd){
                        devnull = open("/dev/null", O_WRONLY);
                        if(devnull == -1){
                            printf("cannot open /dev/null for output\n");
                            fflush(stdout);
                            exit(1);
                        }
                        //Redirect
                        result = dup2(devnull, 0);
                        if(result == -1){
                            perror("dup2 in>/dev/null\n");
                            exit(2);
                        }
                    }
                    //Handle output file
                    if(strcmp(out_file, "") != 0 && out_file != NULL){
                        //Open output file for writing
                        outfile = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        //Check opened correctly
                        if(outfile == -1){
                            printf("cannot open %s for output\n", out_file);
                            fflush(stdout);
                            exit(1);
                        }
                        //Out-file redirection
                        result = dup2(outfile, 1);
                        if(result == -1){
                            perror("dup2 out\n");
                            exit(2);
                        }
                        close(outfile);
                    }
                    
                    //If background process, not in foreground-only mode,
                    //and output file not specified, redirect to /dev/null
                    else if(run_in_backgrnd && !no_backgrnd){
                        devnull = open("/dev/null", O_WRONLY);
                        if(devnull == -1){
                            printf("cannot open /dev/null for input\n");
                            fflush(stdout);
                            exit(1);
                        }
                        //Redirect
                        result = dup2(devnull, 1);
                        if(result == -1){
                            perror("dup2 in>/dev/null\n");
                            exit(2);
                        }
                    }

                    //Execute the command
                    execvp(args[0], args);
                    perror(args[0]);
                    exit(1);
                    break;

                //If fork unsuccessful:
                case -1:
                    perror("fork error");
                    exit(1);
                    break;

                //Handle background/foreground 
                default:
                    //Print background pid
                    if(run_in_backgrnd && !no_backgrnd){
                        printf("background process id is: %d\n", cpid);
                        fflush(stdout);
                    }
                    // -OR- wait for foreground process
                    else{
                        //Block SIGTSTP until waitpid finishes
                        sigprocmask(SIG_BLOCK, &signals, NULL);
                        //Block the parent until the child with given pid
                        //terminates
                        exit_pid = waitpid(cpid, &exit_status, 0);
                        //If waiting and terminated by SIGINT, notify the user
                        if(exit_pid > 0 && WIFSIGNALED(exit_status)){
                            printf("Terminated by signal %d\n", WTERMSIG(exit_status));
                            fflush(stdout);
                        }
                    }
                    //unblock SIGTSTP signals
                    sigprocmask(SIG_UNBLOCK, &signals, NULL);
            }
        }
        //Reset valid input and run_in_backgrnd variables
        valid = false;
        run_in_backgrnd = false;
        
        //Check for background child processes and clean up
        check_background();
    }
    //Clean up containers
    free(user_input);
    free(pid);
    
    return 0;
}


