/*
* Name: Christopher Bugsch
* Assignment: CS344 - Program 3
* Date: 3/5/18
* Description: Program creates a small shell that runs bash
* commands. For a list of commands and how to use the shell
* refer to the attached README file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/wait.h>
#include <signal.h>

char* args[512];		//array of strings to hold input args (max 512 args)
char input[2050];		//string that will hold input from user (max 2048 char plus '\0')
char* inputPtr = input;	//holds address of input string
size_t inputSize = 2048;	//max size to read in for input
int numArgs = 0;		//holds the number of arguments
int status;			//will hold the value from the parent wait
char* savePtr1 = NULL;	//used for calling strtok_r in splitLine
char* savePtr2 = NULL;	//used for calling strtok_r in expandPid
int pidArr[10];		//holds pids of background processes
int fgMode = 0;		//bool to toggle foreground only mode 

struct sigaction SIGINT_action = { 0 }, SIGTSTP_action = { 0 };

/*
* Function: catchSIGSTP
* Inputs: stsp signal
* Outputs: nothing 
* Description: Function waits for any processes to finish,
* then alerts user what mode they are in and toggles that mode on
*/
void catchSIGTSTP(int signo) {
	char* message;
	int childExitMethod;
	
	//wait for child to finish before printing message
	wait(&childExitMethod);
	if (fgMode == 0) {
		message = "Entering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
		fgMode = 1;
	}
	else {
		message = "Exiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		fgMode = 0;
	}
}

/*
* Function: addPid
* Inputs: a pid
* Outputs: nothing 
* Description: Function adds a pid from a background
* child to array at first empty spot
*/
void addPid(int pid) {
	for (int i = 0; i < 10; i++) {
		if (pidArr[i] == 0) {
			pidArr[i] = pid;
			break;
		}
	}
}

/*
* Function: printArgs
* Inputs: array of strings that will be arg values
* Outputs: Nothing
* Description: Function loops through the array of
* args and prints them to the screen. 
* Created as a helper function to be used to development/testing
*/
void printArgs(char* args[512]) {
	for (int i = 0; i < numArgs; i++) {
		printf("Arg #%d: %s, ", i + 1, args[i]);
		fflush(stdout);
	}
}

/*
* Function: expandPid
* Inputs: string containing $$
* Outputs: nothing (function updates the main var)
* Description: Function cuts off the $$ value of a string
* and appends the string value of the current pid to the string
*/
void expandPid(char* string) {
	char pidString[6];			//will hold the string value of pid
	const char* delim = "$$";	//value to cut off

	memset(pidString, '\0', sizeof(pidString));	//set string to null

	int pid = getpid();	//get pid

	sprintf(pidString, "%d", pid);	//convert int to string

	string = strtok_r(string, delim, &savePtr2);	//cut off the $$

	strcat(string, pidString);	//add on the pid
}

/*
* Function: splitLine
* Inputs: string of future args seperated by spaces
* Outputs: nothing (functions adds individual string args to array)
* Description: Function takes a long string and breaks it into individual
* strings that will be used as arguments. If a string contains $$, the pid will be expanded
* inside this function.
*/
void splitLine(char string[2050]) {
	char* token;
	char* pidArg;
	const char* delim = " ";

	//get first argument
	token = strtok_r(string, delim, &savePtr1);

	while (token != NULL) {
		//check if string contained $$
		if (strstr(token, "$$") != NULL) {
			//if argument was only $$
			if (strcmp(token, "$$") == 0) {
				pidArg = malloc(sizeof(char) * 6);	//will hold the string value of pid
				int pid = getpid();	//get pid
				sprintf(pidArg, "%d", pid);	//convert int to string
				args[numArgs] = pidArg;	//save the expanded pid argument
			}
			//if argument was a string with $$ in it
			else {
				pidArg = malloc(sizeof(token));	//allocate space for string
				strcpy(pidArg, token);	//cpy string over
				expandPid(pidArg);	//expand the pid
				args[numArgs] = pidArg;	//save the expanded pid argument
			}
		}
		//if token did not contain $$
		else {
			args[numArgs] = token;	//just save the orig argument
		}
		numArgs++;	// increase the counter
		token = strtok_r(NULL, delim, &savePtr1);	//get next argument
	}
}

/*
* Function: exeCD
* Inputs: string of a dirName 
* Outputs: nothing 
* Description: Function changes directory to the dir
* that was passed in. If no dir was specified, it will 
* attempt to change in the home dir
*/
void exeCD(char* command) {
	//specific directory used
	if (args[1] != NULL) {
		int result = chdir(args[1]);
		//alert on fail
		if (result == -1) {
			perror(args[1]);
		}
	}
	//no directory specified 
	else {
		chdir(getenv("HOME"));
	}
}

/*
* Function: exeExit
* Inputs: None
* Outputs: None
* Description: Function executes the exit command
*/
void exeExit() {
	exit(0);
}

/*
* Function: exeStatus
* Inputs: int child exit method
* Outputs: nothing 
* Description: Function checks the exit or signal status
* of the child passed in and prints the exit or signal value
*/
void exeStatus(int childExitMethod) {
	//if child exited normally 
	if (WIFEXITED(childExitMethod) != 0) {
		printf("exit value %d\n", WEXITSTATUS(childExitMethod));
		fflush(stdout);
	}
	//if child was signaled 
	else if (WIFSIGNALED(childExitMethod) != 0) {
		printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
		fflush(stdout);
	}
}

/*
* Function: isBackground
* Inputs: string argument
* Outputs: int (bool) value 
* Description: Function should be passed the last argument. 
* It then checks to see if that argument was the & symbol. 
* If it does, command is to be run in the background and function
* returns 1. If it is not, command is meant to be run in the foreground
* and function returns 0.
*/
int isBackground(char* arg) {
	//compare the argument to &
	if (strcmp(arg, "&") == 0) {
		return 1;	//command should be run in the background
	}
	else { return 0; }	//command should be run in the foreground
}

/*
* Function: findInput
* Inputs: None (accesses the argument array)
* Outputs: string argument
* Description:Function loops through the array of arguments looking 
* for the '<'  symbol for input redirection. If it finds it, it will 
* return the next argument in the array (which should be a file input)
*/
char* findInput() {
	for (int i = 0; i < numArgs; i++) {
		if (strcmp(args[i], "<") == 0) {
			return args[i + 1];
		}
	}
	return NULL;
}

/*
* Function: findOutput
* Inputs: None (accesses the argument array)
* Outputs: string argument
* Description:Function loops through the array of arguments looking
* for the '>' or '>>' symbol for output redirection. If it finds it, it will
* return the next argument in the array (which should be a file output)
*/
char* findOutput() {
	for (int i = 0; i < numArgs; i++) {
		if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) {
			return args[i + 1];
		}
	}
	return NULL;
}

/*
* Function: removeArgs
* Inputs: an array of all arguments and array to hold slimmed down arguments
* Outputs: None (changes values in slimArgs array)
* Description:Function copies arguments over to another array that will
* not contain redirection or background symbols
*/
void removeArgs(char* allArgs[512], char* slimArgs[512]) {
	char* arg;
	int i = 0; 
	int j = 0;

	while(i < numArgs){
		//if not a redirection command or background symbol
		if (strcmp(allArgs[i], "<") != 0 && strcmp(allArgs[i], ">") && strcmp(allArgs[i], "&")) {
			slimArgs[j] = allArgs[i];
			j++;
			i++;
		}
		//else skip that comman and the next
		else { i += 2; }
	}
	slimArgs[j] = NULL;	//add NULL as last arg
}

/*
* Function: exeOther
* Inputs:string command
* Outputs: Nothing
* Description: Function creates a child process and executes the passed
* in command based on all arguments. 
*/
void exeOther(char* command) {
	pid_t spawnPid = -5;		//child pid
	int childExitMethod = -5;	//child exit method
	int bgBool;				//is background bool
	char* inputSource = NULL;	//file name for input
	char* outputDest = NULL;		//file name for output
	char* passedArgs[512];		//array of string arguments (should not contain redirection args)

	//check if foreground only mode
	if (fgMode == 1) { bgBool = 0; }

	//check if background command
	else{ bgBool = isBackground(args[numArgs - 1]); }

	//begin fork
	spawnPid = fork();

	//child 
	if (spawnPid == 0) {
		int sourceFD, targetFD, result;

		//get input/output source/dest
		inputSource = findInput();
		outputDest = findOutput();

		// ************ BEGIN REDIRECTION ******************* //
		//if running in background and no input was specified 
		if (bgBool == 1 && inputSource == NULL) {	
			sourceFD = open("/dev/null", O_RDONLY); 
		}	
		//if input was specified 
		else if (inputSource != NULL) {
			sourceFD = open(inputSource, O_RDONLY);
		}
		//if no input was specified
		else { sourceFD = 0; }

		//check file could not be opened
		if (sourceFD == -1) { perror("source open()"); exit(1); }

		//if running in background and no output was specified 
		if (bgBool == 1 && outputDest == NULL) {
			targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		}
		//output was specified
		else if (outputDest != NULL){ 
			targetFD = open(outputDest, O_WRONLY | O_CREAT | O_TRUNC, 0644); 
		}
		else { targetFD = 1; }

		//check if file could not be opened
		if (targetFD == -1) { perror("target open()"); exit(1); }
		
		//redirect stdin
		result = dup2(sourceFD, 0);

		//confirm redirection
		if (result == -1) { perror("source dup2()"); exit(2); }

		//redirect stdout
		result = dup2(targetFD, 1);

		//confirm redirection
		if (result == -1) { perror("target dup2()"); exit(2); }

		//printArgs(args); //check if we have the correct arguments

		//remove redirection args
		removeArgs(args, passedArgs);

		//printArgs(passedArgs);	//check if correct arguments were removes 

		//if process is to be run in foreground mode
		if (bgBool == 0) {
			//set the sigint signal to deafult action
			SIGINT_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &SIGINT_action, NULL);

			//ignore SIGTSTP signal
			SIGTSTP_action.sa_handler = SIG_IGN;
			sigaction(SIGTSTP, &SIGTSTP_action, NULL);
		}

		//execute command with proper args
		execvp(command, passedArgs);
		
		//command did not execute
		printf("%s: Command not found\n", command);
		fflush(stdout);
		exit(2);
	}
	//parent
	else {
		//if running in background, print the pid
		if (bgBool == 1) {
			printf("background pid is %jd\n", (intmax_t)spawnPid);

			//add pid to pid array
			addPid(spawnPid);
		}
		//if running in foreground mode
		else
		{
			//wait for child to finish
			waitpid(spawnPid, &childExitMethod, 0); 

			//save the status of the child
			status = childExitMethod;

			//check if child exited by signal
			if (WIFSIGNALED(childExitMethod)) {
				//get the signal number
				int termSignal = WTERMSIG(childExitMethod);

				//if signal is from SIGINT
				if (termSignal == 2) {
					printf("terminated by signal %d\n", termSignal);
					fflush(stdout);
				}
			}
		}//end foreground
	} //end parent 
	
}

/*
* Function: exeCmd
* Inputs:string command
* Outputs: Nothing
* Description: Function calls correct function to handle the command
*/
void execCmd(char* command) {
	//changing directories
	if (strcmp(command, "cd") == 0) {
		exeCD(command);
	}
	//exiting program
	else if (strcmp(command, "exit") == 0) {
		exeExit();
	}
	//getting status of last child process
	else if (strcmp(command, "status") == 0) {
		exeStatus(status);
	}
	//all other commands
	else {
		exeOther(command);
	}
}

/*
* Function: checkBG
* Inputs: None
* Outputs: Nothing
* Description: Function loops through the array of background 
* child processes and checks if they have finished
*/
void checkBG() {
	pid_t pid;
	int childExitMethod;

	//loop through entire array
	for (int i = 0; i < 10; i++) {
		//if index contains a pid
		if (pidArr[i] != 0) {
			//check if it finished
			pid = waitpid(pidArr[i], &childExitMethod, WNOHANG);
			//if it did finish
			if (pid != 0) {
				printf("background pid %d is done. ", pidArr[i]);	//print the pid that finished
				exeStatus(childExitMethod);	//print reason pid finsihed
				pidArr[i] = 0;	//reset that index

			}
		}
	}
}



int main() {
	int charCount;
	
	//set up SIGINT signal to be ignored 
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	sigaction(SIGINT, &SIGINT_action, NULL);

	//set up signal handler for SIGTSTP signal
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	while (1) {
		//promt user
		printf(": ");
		fflush(stdout);

		//get input from user
		charCount = getline(&inputPtr, &inputSize, stdin);

		//check if getline failed
		if (charCount == -1) { clearerr(stdin); }

		//remove trailing newline
		input[strlen(input) - 1] = '\0';

		//split the input into arguments
		splitLine(input);
		
		//printArgs(args);	//verify arguments before executing command

		//call function if not blank line or comment line
		if (numArgs != 0 && args[0][0] != '#'){ 
			execCmd(args[0]);
		}

		//reset args
		for (int i = 0; i < numArgs; i++) {
			args[i] = NULL;
		}
		numArgs = 0;

		//check for completed background processes
		checkBG();
	}

	return 0;
}