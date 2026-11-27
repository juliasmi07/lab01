/*
 * main.c - gshell, the Gaucho Shell
 *
 * This is the only file you edit and submit. main() and the prompt loop
 * are done for you. Every other function below has a comment describing
 * what it does. Work through the checkpoints in the writeup in order:
 *
 *   Checkpoint 1: tokenize, parse_pipeline, and the exit built-in
 *   Checkpoint 2: run_command and run_pipeline for a single command
 *   Checkpoint 3: input and output redirection
 *   Checkpoint 4: pipelines
 */

#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;


/** The longest input line is 100 characters, plus 1 for the '\0' */
#define MAX_LINE 101

/** A 100-character line has at most 50 whitespace-separated tokens */
#define MAX_TOKENS 50

/** A pipeline can never have more commands than the line has tokens */
#define MAX_COMMANDS MAX_TOKENS

/**
 * One command of a pipeline. For example, the line
 *
 *     /bin/sort -r < in.txt | /usr/bin/head -n 3
 *
 * is a pipeline of two commands. The first has argv {"/bin/sort", "-r",
 * NULL}, input_file "in.txt", and output_file NULL.
 */
typedef struct {
    /** The executable path and its arguments, followed by NULL (as execve expects) */
    char *argv[MAX_TOKENS + 1];
    /** The file named after <, or NULL if the command has no input redirection */
    char *input_file;
    /** The file named after >, or NULL if the command has no output redirection */
    char *output_file;
} command_t;

/**
 * Splits `line` into its whitespace-separated tokens and stores a pointer
 * to each one in `tokens`. Changes the contents of `line`.
 *
 * Returns the number of tokens.
 *
 * __attribute__((unused)) stops the compiler from warning that a function
 * is never called. You can remove it once you call the function.
 */
__attribute__((unused))


static int tokenize(char *line, char *tokens[]) { //seperating from white space and grabbing the commands 
    char *ptrsaved;
    int count = 0;

    char *token = strtok_r(line, " \t\n\r\f\v", &ptrsaved);

    while (token != NULL) {

        tokens[count] = token;
        count++;


        token = strtok_r(NULL, " \t\n\r\f\v", &ptrsaved);
    }

    tokens[count] = NULL;

    return count;
}

/**
 * Groups `tokens` into the commands of a pipeline and fills in one
 * command_t per command in `commands`.
 *
 * Returns the number of commands, or -1 if the tokens are not a valid
 * pipeline.
 */
__attribute__((unused))
static int parse_pipeline(char *tokens[], int num_tokens, command_t commands[]) { //puts the tokenized phrases --> commands



    if (num_tokens == 0) {
        return -1;
    }

    int cmd_count = 0;
    int argc = 0;

    commands[cmd_count].input_file = NULL;
    commands[cmd_count].output_file = NULL;

    for (int i = 0; i < num_tokens; i++) {

        if (strcmp(tokens[i], "|") == 0) {

            if (argc == 0) {
                return -1;
            }

            commands[cmd_count].argv[argc] = NULL;

            cmd_count++;

            if (cmd_count >= MAX_COMMANDS) {
                return -1;
            }

            argc = 0;
            commands[cmd_count].input_file = NULL;
            commands[cmd_count].output_file = NULL;

        } else if (strcmp(tokens[i], "<") == 0) {

            if (i + 1 >= num_tokens || commands[cmd_count].input_file != NULL) {
                return -1;
            }

            i++;
            if (strcmp(tokens[i], "|") == 0 ||
                strcmp(tokens[i], "<") == 0 ||
                strcmp(tokens[i], ">") == 0) {
                return -1;
            }

            commands[cmd_count].input_file = tokens[i];

        } else if (strcmp(tokens[i], ">") == 0) {

            if (i + 1 >= num_tokens || commands[cmd_count].output_file != NULL) {
                return -1;
            }

            i++;
            if (strcmp(tokens[i],"|") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], ">") == 0) {
                return -1;
            }

            commands[cmd_count].output_file = tokens[i];

        } else {

            if (argc >= MAX_TOKENS) {
                return -1;
            }

            commands[cmd_count].argv[argc] = tokens[i];
            argc++;
        }
    }

    if (argc == 0) {
        return -1;
    }

    commands[cmd_count].argv[argc] = NULL;

    return cmd_count + 1;

}

/**
 * Starts `cmd` in a child process. The child reads its standard input
 * from `in_fd` and writes its standard output to `out_fd`, unless the
 * command redirects them to a file. Does not wait for the child.
 *
 * Returns the child's process ID, or -1 if no child was started.
 */
__attribute__((unused))
static pid_t run_command(command_t *cmd, int in_fd, int out_fd) {

    (void)in_fd;
    (void)out_fd;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        execve(cmd->argv[0], cmd->argv, environ);

        perror(cmd->argv[0]);
        _exit(127);
    }

    return pid;
}
/**
 * Runs the `num_commands` commands of a pipeline, with each command's
 * standard output connected to the next command's standard input. Then
 * waits for all of them and prints each one's exit status, in order.
 */
__attribute__((unused))

static void run_pipeline(command_t commands[], int num_commands) {
    pid_t pids[MAX_COMMANDS];
    int pipes[MAX_COMMANDS - 1][2];

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        int in_fd = (i == 0) ? STDIN_FILENO : pipes[i - 1][0];
        int out_fd = (i == num_commands - 1)
                         ? STDOUT_FILENO : pipes[i][1];

        pids[i] = run_command(&commands[i], in_fd, out_fd);

        if (pids[i] < 0) {
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            for (int j = 0; j < i; j++) {
                waitpid(pids[j], NULL, 0);
            }
            return;
        }
    }

    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_commands; i++) {
        int status;

        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            continue;
        }

        if (WIFEXITED(status)) {
            printf("exit status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("exit status: %d\n", 128 + WTERMSIG(status));
        }
    }
}


/**
 * Runs one line of input: either the exit built-in, or a pipeline of
 * commands. Prints "Invalid command" to stderr if the line is not a valid
 * pipeline.
 */
static void parse_and_run_command(char *line) {
    // TODO (Checkpoint 1): this is not the correct way to check for the
    // exit command. For example, the line "  exit  " should also exit.

    char *tokens[MAX_TOKENS + 1];
    command_t commands[MAX_COMMANDS];

    int num_tokens = tokenize(line, tokens);

    if (num_tokens == 0) {
        fprintf(stderr, "Invalid command\n");
        return;
    }

    if (num_tokens == 1 && strcmp(tokens[0], "exit") == 0) {
        exit(0);
    }

    int num_commands = parse_pipeline(tokens, num_tokens, commands);

    if (num_commands == -1) {
        fprintf(stderr, "Invalid command\n");
        return;
    }

    run_pipeline(commands, num_commands);
}

int main(void) {
    char line[MAX_LINE];

    while (1) {
        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        // fgets keeps the '\n' at the end of the line. strcspn(line, "\n")
        // returns the index of the first '\n' in line (or of the final '\0'
        // if there is none), so this replaces the newline with '\0'.
        line[strcspn(line, "\n")] = '\0';
        parse_and_run_command(line);
    }

    return 0;
}
