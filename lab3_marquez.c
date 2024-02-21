/*******************************************************
 * Name: Kelsey Marquez
 * Date: February 5th, 2024
 * Course: CIS452-01 Operating Systems Concepts
 * Professor: Dr. Dulimarta
 * Description: The purpose of this program is to 
 * ask the user for a command where it is then parsed 
 * out and executes the command as long as they are 
 * valid inputs. The program will end once the user 
 * quits.
 *******************************************************/
// library headers
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

// macros
#define MAX_LENGTH 10000
#define MAX_ARG 100

int main(int argc, char* argv[]) {
	// variables
	char user_command[MAX_LENGTH];
	char* arg[MAX_ARG];
	char *tok;
	pid_t pid;
	int status;
	int i = 0;

	while(1) {
		// user prompt
		printf("\nPlease enter a command: \n");
		fflush(stdout);

		// checks if user input is valid
		if(fgets(user_command, sizeof(user_command), stdin) == NULL) {
		       	perror("Failure, please try again.");
	       		exit(EXIT_FAILURE);
	 	}
	       	// if user quits, the while loop is exited out of and program ends		
		else if(0 == strcmp(user_command, "quit\n")) {
			exit(1);
		} 
		// user input is parsed using strtok
		else {
			printf("Using strtok() on %s\n", user_command);
			tok = strtok(user_command, " \n");
			i = 0;
			while(tok != NULL) {
				arg[i] = tok;
				printf("[%s]\n", tok);
				tok = strtok(NULL, " \n");
				i++;
			}
			arg[i] = NULL; // verifies that last argument is a NULL
		}
		// begin parent/child processes
		pid = fork();
		
		// fork error checking
		if(pid < 0) {
			perror("Error, fork has failed!");
			exit(1);
		}
		// child process
		else if(pid == 0) {
			// executting user input command and error checking
			if(execvp(arg[0], arg) == -1) {
				perror("Error, execvp failed!");
				exit(EXIT_FAILURE);
			}
			// child process exits before parent waits
			exit(0);
		// parent process
		} else {
			struct rusage usage;

			wait(&status);

			// error checking
			if(getrusage(RUSAGE_CHILDREN, &usage) == -1) {
				perror("Error, gertusage failed!");
				exit(EXIT_FAILURE);
			}

			// prints user CPU time used and involuntary context switches
			printf("User CPU time used: %ld.%06ld\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
			printf("# of involuntary context switches: %ld\n", usage.ru_nivcsw);
		}
		
	}	
	return 0;
}
