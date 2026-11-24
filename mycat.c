/*
 * mycat.c - print files to standard output, like cat
 *
 * Usage: mycat [FILE]...
 *
 * Prints each FILE to standard output, in order. With no FILE arguments,
 * copies standard input to standard output.
 *
 * main() is done for you. Write copy_fd and cat_file using only the system
 * calls open, read, write, and close: no FILE *, printf, fopen, fread,
 * fgets, or fwrite. (perror is fine for error messages.)
 */

#include <fcntl.h>
#include <stdio.h>      // only for perror
#include <unistd.h>

/** How many bytes to read at a time */
#define BUFFER_SIZE 4096

/**
 * Copies everything that can be read from in_fd to out_fd, until read
 * returns 0 (end of file).
 *
 * Returns 0 on success, or -1 if a read or write fails.
 */
static int copy_fd(int in_fd, int out_fd) { //using read, write,
    char buffer[BUFFER_SIZE];
    ssize_t n;
    while ((n = read( in_fd,  buffer, sizeof(buffer))) > 0) {

        ssize_t wrote = 0;
        while (wrote < n) {
            ssize_t w = write( out_fd, buffer + wrote, n - wrote);

            if (w < 0) {
                return -1;
            }
            
            wrote += w;
        }
    }

    if (n < 0) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * Opens the file at path, copies it to standard output with copy_fd, and
 * closes it. If anything fails, prints an error message with perror.
 *
 * Returns 0 on success, or -1 on failure.
 */
static int cat_file(const char *path) { //using open, close.. opening file call cpoy_fd to send it to STDOUT
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror(path); //if/when open fails
        return -1;
    }
    int res = copy_fd(fd, STDOUT_FILENO);

    if (res < 0) {
        perror(path);
    }
    close(fd);

    return res;
}


int main(int argc, char *argv[]) {
    // No file names: copy standard input (fd 0) to standard output (fd 1)
    if (argc == 1) {
        if (copy_fd(STDIN_FILENO, STDOUT_FILENO) < 0) {
            perror("mycat");
            return 1;
        }
        return 0;
    }

    // Print each file in order. If one fails, keep going with the rest,
    // but exit with status 1 at the end.
    int status = 0;
    for (int i = 1; i < argc; i++) {
        if (cat_file(argv[i]) < 0) {
            status = 1;
        }
    }
    return status;
}
