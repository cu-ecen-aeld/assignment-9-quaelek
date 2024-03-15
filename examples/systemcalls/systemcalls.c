#include "systemcalls.h"
#include <stdlib.h> // For system(), exit()
#include <unistd.h> // For execv(), fork(), dup2(), close()
#include <sys/wait.h> // For waitpid()
#include <stdarg.h> // For va_list, va_start, va_end
#include <stdbool.h> // For bool type
#include <fcntl.h> // For open()

bool do_system(const char *cmd)
{
    int status = system(cmd);
    return status == 0; // Return true if command executed successfully
}

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char *command[count + 1];
    for (int i = 0; i < count; i++) {
        command[i] = va_arg(args, char*);
    }
    command[count] = NULL;

    pid_t pid = fork();
    if (pid == -1) {
        // Fork failed
        va_end(args);
        return false;
    } else if (pid == 0) {
        // Child process
        if (execv(command[0], command) == -1) {
            // execv failed
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        va_end(args);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    return true; // To satisfy compiler, actual return is done inside if-else
}

bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char *command[count + 1];
    for (int i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    pid_t pid = fork();
    if (pid == -1) {
        // Fork failed
        va_end(args);
        return false;
    } else if (pid == 0) {
        // Child process
        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            // File open failed
            exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            // dup2 failed
            exit(EXIT_FAILURE);
        }
        close(fd); // Close the file descriptor as it's no longer needed

        if (execv(command[0], command) == -1) {
            // execv failed
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        va_end(args);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
    return true; // To satisfy compiler, actual return is done inside if-else
}
