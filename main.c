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

time_t startTime;
int detach;

int read_command(char* command, char* parameters[]) {
    int noParam = 0;
    detach = 0;

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

    if (strcmp(parameters[noParam-1], "&") == 0) {
        detach = 1;
        parameters[noParam-1] = NULL;
        noParam--;
    }

    return noParam;
}

void sigintHandler() {
    time_t endTime = time(0);
    double d = difftime(endTime, startTime);
    int tmp = (int)d;

    int h = tmp / 3600;
    tmp -= h*3600;

    int m = tmp / 60;
    tmp -= m*60;

    int s = tmp;

    fprintf(stdout, "\nTime elapsed %dh, %dm, %ds (%f)\n", h, m, s, d);
    exit(1);
}

void handleChildExit(int signum)
{
    pid_t pid;
    int   status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        kill(pid, SIGINT);   // Or whatever you need to do with the PID
    }
}


int main(int argc, char *argv[]) {
    int childPid;
    int status;
    char command[MAXLINE];
    char *parameters[MAXLINE];
    int noParams;

    signal(SIGINT, sigintHandler);
    signal(SIGTERM, sigintHandler);

    signal(SIGCHLD, handleChildExit);

    // save start time
    startTime = time(NULL); // Get the system time

    while (1) {
        noParams = read_command(command, parameters); // read user input
        if (noParams == 0) {
            fprintf(stderr, "no command ?!\n");
            exit(1);
        }

        if (strcmp(command, "cd") == 0) {
            if (chdir(parameters[1]) == -1) {
                fprintf(stdout, "The specified directory is invalid!\n");
            }
            continue;
        }

        if ((childPid = fork()) == -1) { // create process
            fprintf(stderr, "can't fork!\n");
            exit(2);
        } else if (childPid == 0) { // child process
            execvp(command, parameters); // executes command
            exit(3);
        } else { // father
            if (detach == 0) {
                waitpid(childPid, &status, WUNTRACED | WCONTINUED);
            } else {
                fprintf(stdout, "Child pid: %d\n", childPid);
            }
        }
    }

    exit(0);
}

