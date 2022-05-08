#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>		// open()
#include <signal.h>			// signal()
#include <fcntl.h>			// open()
#include <stdio.h>			// printf(), ...
#include <time.h>				// time(), ...
#include <string.h>			// strtok()
#include <unistd.h>
#define MAXLINE 100
#define MOD "exit with CTR C"

int read_command(char* command, char* parameters[]) {
    int noParam = 0;

    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    fprintf(stdout, "%s > ", cwd);

    char buffer[MAXLINE];
    char *b = buffer;
    size_t bufsize = MAXLINE;
    size_t characters;
    characters = getline(&b,&bufsize,stdin);

    if ((buffer)[characters - 1] == '\n') {
        (buffer)[characters - 1] = '\0';
    }

    char* token = strtok(buffer, " ");
    if (token == NULL) return 0; // kein command eingegeben
    strcpy(command, token);
    while (token != NULL) {
        parameters[noParam] = strdup(token);
        noParam++;
        token = strtok(NULL, " ");
    }
    parameters[noParam] = NULL;


    return noParam;
}


int main(int argc, char *argv[]) {
    int childPid;
    int status;
    char command[MAXLINE];
    char *parameters[MAXLINE];
    int noParams;

    while (1) {
        noParams = read_command(command, parameters); // read user input
        if (noParams == 0) {
            fprintf(stderr, "no command ?!\n");
            exit(1);
        }

        if ((childPid = fork()) == -1) { // create process
            fprintf(stderr, "can't fork!\n");
            exit(2);
        } else if (childPid == 0) { // child process
            execvp(command, parameters); // executes command
            exit(3);
        } else { // father
            waitpid(childPid, &status, WUNTRACED | WCONTINUED);
        }
    }

    exit(0);
}

