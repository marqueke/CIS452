/*******************************************************************************
 * Name: Kelsey Marquez
 * Description: The purpose of this program is to recall knowledge from C 
 * programming to find certain information about the user and operating system.
 * Assignment: Lab02 C Warmup
 * Professor: Dr. Duilimarta
 * Course: CIS452 Operating Systems Concepts
 *******************************************************************************/

/*** HEADER FILES ***/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/stat.h>

/*** INITIALIZING FUNCTIONS ***/
int aboutUserID();
int myMachine();
int aboutMe();


int main(int arg, char *arr[]) {
	printf("About Me\n");
        printf("====================\n");

	// retrieves filepath of current file
	const char *file = arr[0];	
	struct stat file_stat;

	// uses stat() to retreive file information
	if(stat(file, &file_stat) == 0) {
		mode_t permissions = file_stat.st_mode;

		// printing permissions in octal format
		printf("Octal Permissions: %o\n", permissions);

		char per_str[10];

		// converting permissions to text format
		snprintf(per_str, sizeof(per_str), "%c%c%c%c%c%c%c%c%c",
				(permissions & S_IRUSR) ? 'r' : '-',
				(permissions & S_IWUSR) ? 'w' : '-',
				(permissions & S_IXUSR) ? 'x' : '-',
				(permissions & S_IRGRP) ? 'r' : '-',
				(permissions & S_IWGRP) ? 'w' : '-',
				(permissions & S_IXGRP) ? 'x' : '-',
				(permissions & S_IROTH) ? 'r' : '-',
				(permissions & S_IWOTH) ? 'w' : '-',
				(permissions & S_IXOTH) ? 'x' : '-');

		// printing permissions to text format
		printf("Text Permissions: %s\n", per_str);
	} else {
		// if not valid, exit
		perror("Error getting file information.\n");
		return EXIT_FAILURE;
	}

	aboutUserID();
	aboutMe();
	myMachine();

	printf("\n\n\n");

	return 0;
}


int aboutMe() {
	uid_t uid;
	gid_t gid;

	// gets real user ID
	uid = getuid();
	// gets group user ID
	gid = getgid();
	
	// will be used to return specific members in the password struct
	struct passwd *info = getpwuid(uid);
	// will be used to return specific members in the group struct
	struct group *grp = getgrgid(gid);

	// prints user's full name 
	printf("Name\t\t: %s\n", info->pw_gecos);
	// prints Unix group name
	printf("Unix Group\t: %s \n", grp->gr_name);
	// prints home path of user
	printf("Unix Home\t: %s\n", info->pw_dir);
	// prints login shell of operating system machine
	printf("Login Shell\t: %s \n", info->pw_shell);
	printf("\n");	
	
	return 0;
}

int aboutUserID() {
	// sample code used from Lab2 handout
	char *username;
	username = getenv("USER");
	pid_t unixID;

	// returns the process ID of Unix
	unixID = getpid();

	// prints username of user
	printf("Unix User\t: %s\n", username);
	// prints Unix UID
	printf("Unix ID\t\t: %d\n", unixID);

	return 0;
}

int myMachine() {
	struct utsname unameData;
	
	// will be used to display certain information in the utsname struct
	uname(&unameData);

	printf("\nAbout My Machine\n");
	printf("====================\n");
	// prints the host of the machine
	printf("Host\t\t: %s \n", unameData.nodename);
	// prints the system of the machine as well as it's system release
	printf("System\t\t: %s %s\n", unameData.sysname, unameData.release);

	return 0;
}
