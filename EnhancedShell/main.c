/*********************************************************************
 * Name: Kelsey Marquez
 * Date: February 17th, 2024
 * Course: CIS452-01 Operating Systems Concepts
 * Professor: Dr. Dulimarta
 * Assignment: Enhanced Shell (Individual Assignment)
 * Description: The purpose of this program is to ask the user for a 
 * valid command then parse out and execute the command. The user can 
 * input simple commands or type a sequence of commands in a pipe 
 * chain, there can be directioning of a file and overwrites, keep a 
 * history of successfully executed commands in proceeding and reverse 
 * order, and allow the user to redo a command. The program will end 
 * once the user quits. 
 **********************************************************************/
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
#include <error.h>
#include <fcntl.h>
#include <ctype.h>

// macros
#define MAX_LENGTH 10000
#define MAX_ARG 100
#define MAX_HISTORY 10
#define READ 0
#define WRITE 1


void file_redirection(char* command);
void command_history(const char* command);
void print_history();
void execute_history(int history_num);
void execute_command(char* user_command);

// global variables
char* history[MAX_HISTORY] = {NULL};
int count = 0;

int main(int argc, char* argv[]) {
	char user_command[MAX_LENGTH];
	char* args[MAX_ARG];
	int fd[2];		// pipes
	int prev_fd, i;
	pid_t pid;

	while(1) {
		// user prompt
		printf("\n[1] Enter a pipe chain of commands separated by a '|'. \n");
		printf("[2] Read the input from a .txt file by typing the command followed by a '<', then the name of the .txt file.\n");
		printf("[3] Save the command text to a file by typing the command followed by a '>', then the name of the .txt file.\n");
	       	printf("[4] Overwrite the contents of a file by typing the command followed by a '>!', then the name of the .txt file.\n");
		printf("[5] Type 'history' to view the history of commands in reverse order.\n");
		printf("[6] Type 'quit' to quit the program.\n");
		printf("\n[TYPE HERE]: \n");

		fflush(stdout);

		// checks if user input is valid
		if(fgets(user_command, sizeof(user_command), stdin) == NULL) {
		       	perror("Failure, please try again.");
	       		//continue;
			exit(EXIT_FAILURE);
 	 	}

		user_command[strcspn(user_command, "\n")] = '\0';

		if(strcmp(user_command, "history") == 0) {
			print_history();
			continue;
		} else if(user_command[0] == '!') {
			int history_num = atoi(&user_command[1]);
			if(history_num > 0 && history_num <= count) {
				execute_history(history_num);
				continue;
			} else {
				printf("Invalid history reference '%s'\n", user_command);
				continue;
			}
		} else if(strcmp(user_command, "quit") == 0) {
			break;
		}

		command_history(user_command);

		int num_comm = 1;
		for(i = 0; user_command[i]; i++) {
			if(user_command[i] == '|') {
				num_comm++;
			}
		}

		char* token = strtok(user_command, "|");
		i = 0;
		while(token != NULL) {
			args[i++] = token;
			token = strtok(NULL, "|");
		}
		args[i] = NULL;	// end of num commands

		prev_fd = 0;	// use STDIN for first command
		
		for(i = 0; i < num_comm; i++) {
			// create pipes except for last command
			if(i < num_comm - 1) {
				if(pipe(fd) < 0) {
					perror("Pipe Error.");
					exit(EXIT_FAILURE);
				}
			}

			pid = fork();
 			
			// fork error checking
			if(pid < 0) {
				perror("Error, fork has failed!");
				exit(EXIT_FAILURE);
			}
			/********** CHILD PROCESS **********/
			else if(pid == 0) {
				if(prev_fd != 0) {
					dup2(prev_fd, STDIN_FILENO);
					close(prev_fd);
				}

				// redirect output to next command
				if(i < num_comm - 1) {
					close(fd[READ]);
					dup2(fd[WRITE], STDOUT_FILENO);
					close(fd[WRITE]);
					}

				// handling redirection
				file_redirection(args[i]);

				// parse and execute commands
				char* arr[MAX_ARG];
				char* token = strtok(args[i], " \t");
				int j = 0;
				while(token != NULL) {
					arr[j++] = token;
					token = strtok(NULL, " \t");
				}	
				arr[j] = NULL;

				// executting user input command and error checking
				if(execvp(arr[0], arr) < 0) {
					perror("Error, execvp failed!");
					exit(EXIT_FAILURE);
				}
				exit(EXIT_SUCCESS);
			/********** PARENET PROCESS **********/
			} 
			else {
				if(prev_fd != 0) {
					close(prev_fd);
				}
				if(i < num_comm - 1) {
					close(fd[WRITE]);
					prev_fd = fd[READ];
				}
				wait(NULL);	// wait for child process to finish
			}
		}
		
	}
	
	// free allocated memory
	for(int i = 0; i < count; i++) {
		free(history[i]);
	}

	return 0;
}

void file_redirection(char* command) {
	char *file_in, *file_out, *force_out;
	int input, output;

	// reset pointers to NULL at start
	file_in = NULL;
	file_out = NULL;
	force_out = NULL;

	/********** INPUT TXT FILE **********/
	file_in = strstr(command, "<");
	if(file_in) {
		*file_in = 0;		// terminate command part 
		file_in++;		// move to beginning of filename
					
		while(*file_in && isspace((unsigned char) *file_in)) {
			file_in++;	// skip leading spaces
		}

		if(access(file_in, F_OK) != 0) {
			fprintf(stderr, "[ERROR. INPUT FILE DOES NOT EXIST.]\n");
		}
		
		// create read only file
		input = open(file_in, O_RDONLY);

		if(input < 0) {
			perror("[FAILED TO OPEN INPUT FILE]");
			exit(EXIT_FAILURE);
		}
		dup2(input, STDIN_FILENO);
		close(input);
	}

	/********** OUTPUT TXT FILE **********/
	file_out = strstr(command, ">");
	if(file_out && file_out[1] != '!') {	// do not do overwrite
		*file_out = 0;		// terminate command part
		file_out++;		// move to beginning of filename
		
		while(*file_out && isspace((unsigned char) *file_out)) {
			file_out++;	// skip leading spaces
		}

		if(access(file_out, F_OK) == 0) {
			fprintf(stderr, "[ERROR. OUTPUT FILE ALREADY EXISTS. USE '>!' IF YOU WISH TO OVERWRITE THE FILE.]\n");
		}

		// 0_TRUNC to overwrite if file exists, 0_CREAT to create if it doesn't exist
		output = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		
		if(output < 0) {
			perror("[FAILED TO OPEN OUTPUT FILE]");
			exit(EXIT_FAILURE);
		}
		dup2(output, STDOUT_FILENO);
		close(output);
	}
	else {
	/********** FORCED OUTPUT TXT FILE **********/
		force_out = strstr(command, ">!");
		if(force_out) {
			*force_out = 0;		// terminate command part
			force_out += 2;		// move to beginning of filename

			while(*force_out && isspace((unsigned char) *force_out)) {
				force_out++;	// skip leading spaces
			}
	
			output = open(force_out, O_WRONLY | O_CREAT | O_TRUNC, 0666);

			if(output < 0) {
				perror("[FAILED TO OPEN OUTPUT FILE.]");
				exit(EXIT_FAILURE);
			}
			dup2(output, STDOUT_FILENO);
			close(output);
		}
	}
}


void command_history(const char* command) {
	// free oldest command if max is full
	if(count == MAX_HISTORY) {
		free(history[0]);
		for(int i = 1; i < MAX_HISTORY; i++) {
			history[i-1] = history[i];
		}
		count--;
	}
	history[count++] = strdup(command);
}


void print_history() {
	printf("/********** HISTORY OF COMMANDS **********/\n");
	for(int i = 0; i < count; i++) {
		printf("[%d] %s\n", i+1, history[i]);
	}
}


void execute_history(int history_num) {
	if(history_num < 1 || history_num > count) {
		printf("[ERROR. COMMAND IS NOT IN HISTORY.]\n");
		return;
	}
	// extract command from history
	char* command = history[history_num - 1];
	printf("Executing command: %s\n", command);
	
	execute_command(command);
	free(command);

}

void execute_command(char* user_command) {
    char* args[MAX_ARG];
    int fd[2];
    int prev_fd = 0;
    pid_t pid;
    int num_comm = 0;

    for (int i = 0; user_command[i]; i++) {
        if (user_command[i] == '|') num_comm++;
    }
    num_comm++; // Adjust for actual number of commands

    char* token = strtok(user_command, "|");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, "|");
    }
    args[i] = NULL; // End of commands

    for (i = 0; i < num_comm; i++) {
        if (i < num_comm - 1) {
            if (pipe(fd) < 0) {
                perror("Pipe Error.");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();

        if (pid < 0) {
            perror("Error, fork has failed!");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            if (prev_fd != 0) {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if (i < num_comm - 1) {
                close(fd[READ]);
                dup2(fd[WRITE], STDOUT_FILENO);
                close(fd[WRITE]);
            }

            file_redirection(args[i]);

            char* arr[MAX_ARG];
            char* command_token = strtok(args[i], " \t");
            int j = 0;
            while (command_token != NULL) {
                arr[j++] = command_token;
                command_token = strtok(NULL, " \t");
            }
            arr[j] = NULL;

            if (execvp(arr[0], arr) < 0) {
                perror("Error, execvp failed!");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        } else { // Parent process
            if (prev_fd != 0) {
                close(prev_fd);
            }
            if (i < num_comm - 1) {
                close(fd[WRITE]);
                prev_fd = fd[READ];
            }
            wait(NULL); // Wait for child process to finish
        }
    }
}
