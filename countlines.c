/*
 * countlines.c - run a command and count the lines it prints
 *
 * Usage: countlines COMMAND [ARG]...
 *
 * Runs COMMAND with its standard output connected to a pipe, counts the
 * lines that come through the pipe, waits for COMMAND to finish, and prints
 *
 *     lines: N
 *     exit status: S
 *
 * If COMMAND can't be run, the child prints an error with perror and exits
 * with status 127 (what a shell uses for "command not found").
 *
 * You may use execvp in this lab, so COMMAND can be a name like ls instead
 * of a full path like /bin/ls.
 *
 * STEP 4 of the writeup walks through how pipe, fork, dup2, and execvp fit
 * together, with a figure of both processes' fd tables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/** How many bytes to read at a time */
#define BUFFER_SIZE 4096

/**
 * Creates a pipe and starts the command argv[0] (with arguments argv) in a
 * child process whose standard output is the pipe's write end. Stores the
 * pipe's read end in *read_fd.
 *
 * Returns the child's process ID, or -1 on error.
 *
 * Hints:
 * - If pipe or fork fails, there is no child yet. Print an error with perror
 *   and return -1.
 * - The child should never return from this function. If it did, it would
 *   go back to main and keep running as a second copy of countlines. So if
 *   dup2 or execvp fails, print an error with perror and exit. (execvp only
 *   returns when it fails.)
 * - After dup2, the child's standard output is the pipe, so anything the
 *   child prints with printf goes into the pipe and gets counted. Print
 *   debugging messages to stderr instead: fprintf(stderr, ...).
 *
 * __attribute__((unused)) stops the compiler from warning that a function
 * is never called. You can remove it once you call the function.
 */
__attribute__((unused))
static pid_t start_command(char *argv[], int *read_fd) {

    int fds[2];

    if (pipe(fds) < 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        close(fds[0]);
        close(fds[1]);

        return -1;
    }

    if (pid == 0) {
        if (dup2(fds[1], STDOUT_FILENO) < 0) {
            perror("dup2");
            _exit(1);
        }
        close(fds[0]);
        close(fds[1]);

        execvp(argv[0], argv);
        perror(argv[0]);  
        _exit(127);
    }

    close(fds[1]);
    *read_fd = fds[0];
    return pid;
}

/**
 * Reads from fd until end of file.
 *
 * Returns the number of newline characters read.
 *
 * Hint: this is the same read loop as copy_fd in mycat. read can return fewer
 * bytes than the buffer holds, so only look at the bytes it returned.
 */
__attribute__((unused))
static int count_lines(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t n;
    int lines = 0;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buffer[i] == '\n') {
                lines++;
            }
        }
    }
    return lines;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: countlines COMMAND [ARG]...\n");
        return 1;
    }

    // Start the command, count the lines it prints, wait for it to finish,
    // and print the two lines.
    //
    // start_command expects the command's own argument list, where argv[0]
    // is the command's name (like "echo") and the rest are its arguments.
    // Here in main, argv[0] is "./countlines". Which part of main's argv is
    // the command?
    //
    // Count the lines *before* you wait. A pipe holds only about 64 KB. Once
    // it is full, the command's next write blocks until someone reads. If the
    // parent is sitting in waitpid instead of reading, the command never
    // finishes, and both processes wait for each other forever.
    //
    // The status that waitpid gives you packs several things into one int
    // (for example, whether the child exited normally or was killed by a
    // signal). Use the WEXITSTATUS macro from <sys/wait.h> to get the exit
    // status out of it.

    int read_fd;
    pid_t pid = start_command(argv + 1, &read_fd);
    if (pid < 0) {
        return 1;
    }

    int lines = count_lines(read_fd);   // read everything BEFORE waiting
    close(read_fd);

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    printf("lines: %d\n", lines);
    printf("exit status: %d\n", WEXITSTATUS(status));
    return 0;

}
