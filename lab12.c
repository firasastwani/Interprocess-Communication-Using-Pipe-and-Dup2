#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Main function that implements pipe-based interprocess communication
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Exit status of the program
 */
int main(int argc, char *argv[]) {
    // Check if we have enough arguments
    if (argc < 4) {
        printf("Usage: %s <command1> [args1] -pipe <command2> [args2]\n", argv[0]);
        return 1;
    }

    // Find the -pipe flag position
    int pipe_pos = -1;
    bool found_pipe = false;
    for (int i = 1; i < argc && !found_pipe; i++) {
        if (strcmp(argv[i], "-pipe") == 0) {
            pipe_pos = i;
            found_pipe = true;
        }
    }

    if (!found_pipe) {
        printf("Error: -pipe flag not found\n");
        return 1;
    }

    // Create pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    // Fork process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // Child process
        // Close read end of pipe
        close(pipefd[0]);
       
        // Redirect stdout to write end of pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            return 1;
        }
        close(pipefd[1]);

        // Prepare command for child process
        char *child_argv[pipe_pos];
        for (int i = 0; i < pipe_pos; i++) {
            child_argv[i] = argv[i + 1];
        }
        child_argv[pipe_pos - 1] = NULL;

        // Execute child command
        execvp(child_argv[0], child_argv);
        perror("execvp");
        return 1;
    } else { // Parent process
        // Close write end of pipe
        close(pipefd[1]);
        
        // Redirect stdin to read end of pipe
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            return 1;
        }
        close(pipefd[0]);

        // Wait for child to finish
        int status;
        if (wait(&status) == -1) {
            perror("wait");
            return 1;
        }

        // Prepare command for parent process
        char *parent_argv[argc - pipe_pos];
        for (int i = 0; i < argc - pipe_pos - 1; i++) {
            parent_argv[i] = argv[pipe_pos + i + 1];
        }
        parent_argv[argc - pipe_pos - 1] = NULL;

        // Execute parent command
        execvp(parent_argv[0], parent_argv);
        perror("execvp");
        return 1;
    }

    return 0;
}
