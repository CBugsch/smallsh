* Name: Christopher Bugsch
* Assignment: CS344 - Program 3
* Date: 3/5/18
* Description: Program creates a small shell that runs bash commands

COMPILE: To compile the program, use the attached build script and run the command 'build'

EXECUTE: To execute the program, run the command 'smallsh'

COMMANDS: To run a command during the program, enter the command with any included
	arguments, each seperated by a space. If no arguments are given or you use the '#' comment
	symbol at the begining of the command, that command will be ignored and you will be repromted.

	To exit the program, run 'exit' command when prompt is available. Program can not be terminted 
	using ^C or ^Z. 

	To run a command in the background, add '&' as the last argument of your command.
	To turn off background processes, press ^Z. Press again to turn back on.

	Pressing ^C will terminate only processes running in the foreground, but as mentioned, will
	not terminate the shell itself or any background processes.

COMMAND LIST:
	This shell has been tested to execute the following commands:
	ls
	cd
	pwd
	cat
	wc
	status
	sleep
	echo
	exit
	kill
	date

Although other commands should work, they have not been tested and cannot be guaranteed to function
properly. If you are having trouble running a command, first make sure you have a space between the
command and every single argument. If you receive an error, make sure you are passing the command the 
proper arguments. 



