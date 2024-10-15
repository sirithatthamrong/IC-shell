/* ICCS227: Project 1: icsh
 * Name: Kanladaporn Sirithatthamrong
 * StudentID: 6480952
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

#define RESET_COLOR "\033[0m"
#define BOLD        "\033[1m"
#define UNDERLINE   "\033[4m"

#define TEXT_COLOR_RED    "\033[31m"
#define TEXT_COLOR_GREEN  "\033[32m"
#define TEXT_COLOR_YELLOW "\033[33m"
#define TEXT_COLOR_BLUE   "\033[34m"
#define TEXT_COLOR_CYAN   "\033[36m"
#define TEXT_COLOR_WHITE  "\033[37m"

#define MAX_CMD_BUFFER 255
#define MAX_LINE_LENGTH 255
#define MAX_JOBS 100
#define MAX_PATH 1024

typedef enum JobStatus {
    RUNNING = 0,
    DONE = 1,
    STOPPED = -1
} JobStatus;

typedef struct Job {
    int id;         // Job ID
    pid_t pid;      // Process ID
    char* command;  // Command string
    JobStatus status;     // Job status
} Job;

/* Function declarations */
void displayWelcomeMessage();
void printHelp();
char** tokenizeInput(const char* input);
void freeTokens(char** tokens);
char** copyTokens(char** tokens);
void printTokens(char** tokens, int start);
char* tokensToStr(char** tokens, int from);
void handleSigint(int sig);
void handleSigtstp(int sig);
void handleSigchld(int sig);
void setupSignal(int signal, void (*handler)(int));
void setupSignalHandlers();
int setupRedirection(char** args);
void addJob(pid_t pid, char* command, int status);
void removeJob(pid_t pid);
void updateJobStatus(pid_t pid, int status);
void displayJob(const Job* job, char symbol);
void listJobs();
void stopAllOtherJobs(pid_t excludePid);
Job* findJobById(int id);
void handleEcho(char** current);
void handleExit(char** current);
void handleFg(char** current);
void handleBg(char** current);
void executeExternalCommand(char** current, int isBackground);
void handleCd(char** current);
void handlePipe(char** firstCmd, char** secondCmd);
int hasSpecialToken(char** current);
void handleCommand(char** current, char** previous);
void readScripts(const char* fileName);

/* Global variables */
char** prevCommandArgs = NULL;
char** lastCommandArgs = NULL;
char** currCommandArgs = NULL;
char prompt[MAX_CMD_BUFFER] = "icsh $ ";
int lastExitStatus = 0;
pid_t foregroundPid = 0;
Job jobs[MAX_JOBS];
int jobCount = 0;

/* Display the welcome message */
void displayWelcomeMessage() {
    printf(TEXT_COLOR_CYAN BOLD "\n╭─────────────────────────────────────────────────────────────────────────╮\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│" RESET_COLOR TEXT_COLOR_GREEN BOLD "                   🌟 Welcome to the IC Shell! 🌟                        " RESET_COLOR TEXT_COLOR_CYAN BOLD"│\n");
    printf(TEXT_COLOR_CYAN BOLD   "│" RESET_COLOR TEXT_COLOR_WHITE "          Type 'help' to see the list of available commands              " RESET_COLOR TEXT_COLOR_CYAN BOLD"│\n");
    printf(TEXT_COLOR_CYAN BOLD   "╰─────────────────────────────────────────────────────────────────────────╯\n" RESET_COLOR);
    printf("\n");
}

/* Print the help menu */
void printHelp() {
    printf(TEXT_COLOR_CYAN BOLD "\n╭──────────────────────────── " RESET_COLOR "Help Menu" TEXT_COLOR_CYAN BOLD " ──────────────────────────────────────────╮\n");
    printf(TEXT_COLOR_CYAN BOLD   "│ " TEXT_COLOR_BLUE "Command" TEXT_COLOR_CYAN BOLD"                         │  " TEXT_COLOR_BLUE "Description" TEXT_COLOR_CYAN BOLD "                                  │\n");
    printf(TEXT_COLOR_CYAN BOLD   "├─────────────────────────────────┼───────────────────────────────────────────────┤\n");

    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "help                          " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Display this help menu                       " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "cd <dir>                      " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Change the current directory                 " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "echo <text>                   " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Print the given text to the console          " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "!!                            " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Repeat the last command                      " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "exit <num>                    " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Exit the shell with a given exit code        " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "./icsh <script_file>          " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Execute a script file                        " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "> <file>                      " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Redirect output to a file                    " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "< <file>                      " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Redirect input from a file                   " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "<command> &                   " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Run a command in the background              " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "jobs                          " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  List all background jobs                     " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "fg %<job_id>                  " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Bring a background job to the foreground     " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "bg %<job_id>                  " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Resume a stopped background job              " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);
    printf(TEXT_COLOR_CYAN BOLD   "│ " RESET_COLOR "> " TEXT_COLOR_YELLOW BOLD "<command1> | <command2>       " RESET_COLOR TEXT_COLOR_CYAN BOLD "│" RESET_COLOR "  Pipe the output of command1 to command2      " TEXT_COLOR_CYAN BOLD "│\n" RESET_COLOR);

    printf(TEXT_COLOR_CYAN BOLD   "╰─────────────────────────────────────────────────────────────────────────────────╯\n" RESET_COLOR);
    printf("\n");
}

/* Tokenize the input string and return an array of tokens */
char** tokenizeInput(const char* input) {
    char** tokens = malloc(MAX_CMD_BUFFER * sizeof(char*));
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    char* token;
    char* inputCopy = strdup(input);
    int i = 0;

    // Tokenization logic that handles quotes properly
    token = strtok(inputCopy, " \t\n");
    while (token != NULL) {
        if (token[0] == '"' && token[strlen(token) - 1] == '"') {
            // Remove the quotes if present
            token[strlen(token) - 1] = '\0';
            tokens[i] = strdup(token + 1);
        } else {
            tokens[i] = strdup(token);
        }
        token = strtok(NULL, " \t\n");
        i++;
    }
    tokens[i] = NULL;
    free(inputCopy);
    return tokens;
}

/* Free the memory allocated for the tokens */
void freeTokens(char** tokens) {
    if (tokens) {
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
}

/* Copy the tokens array and return a new array */
char** copyTokens(char** tokens) {
    if (!tokens) return NULL;

    char** copy = malloc(MAX_CMD_BUFFER * sizeof(char*));
    if (!copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (tokens[i] != NULL) {
        copy[i] = strdup(tokens[i]);
        if (!copy[i]) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        i++;
    }
    copy[i] = NULL;
    return copy;
}

/* Print the tokens array starting from the given index */
void printTokens(char** tokens, int start) {
    for (int i = start; tokens[i] != NULL; i++) {
        printf("%s ", tokens[i]);
    }
    printf("\n");
}

/* Concatenate the tokens array starting from the given index */
char* tokensToStr(char** tokens, int from) {
    char* result = calloc(MAX_LINE_LENGTH, sizeof(char));
    if (!result) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    for (int i = from; tokens[i] != NULL; i++) {
        strcat(result, tokens[i]);
        strcat(result, " ");
    }

    return result;
}

/* Handle the SIGINT (Ctrl+C) */
void handleSigint(int sig) {
    if (foregroundPid > 0) {
        kill(foregroundPid, SIGINT);
        updateJobStatus(foregroundPid, DONE);
    }
}

/* Handle the SIGTSTP (Ctrl+Z) */
void handleSigtstp(int sig) {
    if (foregroundPid > 0) {
        // printf("\nPID: %d\n", foregroundPid);
        kill(foregroundPid, SIGTSTP); // Stop the foreground process

        for (int i = 0; i < jobCount; i++) {
            if (jobs[i].pid == foregroundPid) {
                updateJobStatus(foregroundPid, STOPPED);
                break;
            }
        }
    }
}

/* Handle the SIGCHLD (Child process terminated) */
void handleSigchld(int sig) {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < jobCount; i++) {
            if (jobs[i].pid == pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    printf("\n[%d]+ Done                    %s\n", jobs[i].id, jobs[i].command);
                    updateJobStatus(pid, DONE);
                    removeJob(pid);
                } 
                break;
            }
        }
    }
}

/* Seting up signal */
void setupSignal(int signal, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(signal, &sa, NULL);
}

/* Setting up signal handlers */
void setupSignalHandlers() {
    setupSignal(SIGINT, handleSigint);
    setupSignal(SIGTSTP, handleSigtstp);
    setupSignal(SIGCHLD, handleSigchld);
}

/* Redirect the input/output of the command */
int setupRedirection(char** args) {
    int i = 0;
    while (args[i] != NULL) {
        /* Output redirection */
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "icsh: syntax error near unexpected token 'newline'\n");
                return -1;
            }
            int out = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (out == -1) {
                perror("icsh: open");
                return -1;
            }
            dup2(out, STDOUT_FILENO);
            close(out);
            args[i] = NULL;  // Remove the redirection token from the args
        } 
        /* Input redirection */
        else if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "icsh: syntax error near unexpected token 'newline'\n");
                return -1;
            }
            int in = open(args[i + 1], O_RDONLY);
            if (in == -1) {
                perror("icsh: open");
                return -1;
            }
            dup2(in, STDIN_FILENO);
            close(in);
            args[i] = NULL;  // Remove the redirection token from the args
        }
        i++;
    }
    return 0;
}

/* Add a new job to the job list */
void addJob(pid_t pid, char* command, int status) {
    if (jobCount >= MAX_JOBS) {
        fprintf(stderr, "icsh: maximum number of jobs reached\n");
        return;
    }
    jobs[jobCount] = (Job){ .id = jobCount + 1, 
                            .pid = pid, 
                            .command = strdup(command), 
                            .status = status };
    jobCount++;
}

/* Remove a job from the job list */
void removeJob(pid_t pid) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].pid == pid) {
            free(jobs[i].command); // Free the command memory
            for (int j = i; j < jobCount - 1; j++) {
                jobs[j] = jobs[j + 1]; // Shift the jobs array
            }
            jobCount--;
            break;
        }
    }
}

/* Update the status of the job */
void updateJobStatus(pid_t pid, int status) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].status = status;
            break;
        }
    }
}

/* Display a job with its status */
void displayJob(const Job* job, char symbol) {
    const char* statusStr = (job->status == RUNNING) ? "Running" :
                            (job->status == STOPPED) ? "Stopped" : "Done";
    printf("[%d]%c %s                 %s\n", job->id, symbol, statusStr, job->command);
}

/* List all the jobs in the job list */
void listJobs() {
    int plusIdx = jobCount > 0 ? jobCount - 1 : -1;
    int minusIdx = jobCount > 1 ? jobCount - 2 : -1;

    for (int i = 0; i < jobCount; i++) {
        char symbol = (i == plusIdx) ? '+' : (i == minusIdx) ? '-' : ' ';
        displayJob(&jobs[i], symbol);
    }
}

/* Stop all other jobs except the one with the given PID */
void stopAllOtherJobs(pid_t excludePid) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].pid != excludePid && jobs[i].status == RUNNING) {
            kill(jobs[i].pid, SIGSTOP);  // Stop all other jobs
            jobs[i].status = STOPPED;    // Update their status to stopped
        }
    }
}

/* Find a job by its ID */
Job* findJobById(int id) {
    for (int i = 0; i < jobCount; i++) {
        if (jobs[i].id == id) { 
            return &jobs[i];
        }
    }
    return NULL;
}

/* Handle the echo command */
void handleEcho(char** current) {
    if (strcmp(current[1], "$?") == 0 && current[2] == NULL) {
        printf("%d\n", lastExitStatus);
    } else {
        printTokens(current, 1);
    }
    lastExitStatus = 0;
}

/* Handle the exit command */
void handleExit(char** current) {
    if (current[1] == NULL) {
        printf("$ echo $?\n0\n$\n");
    } else {
        printf("$ echo $?\n%s\n$\n", current[1]);
    }
    int exitCode = current[1] ? atoi(current[1]) : 0;
    exit(exitCode & 0xFF);  // Limit exit code to 0-255
}

/* Handle the fg command */
void handleFg(char** current) {
    if (current[1][0] == '%') {
        int jobId = atoi(&current[1][1]);
        Job* job = findJobById(jobId);
        if (job != NULL && job->status != DONE) {
            printf("%s\n", job->command);
            foregroundPid = job->pid;
            
            stopAllOtherJobs(job->pid);
            if (job->status == STOPPED) { // Send continue signal if the job was stopped
                kill(job->pid, SIGCONT);
            }

            // Wait for the job to finish in the foreground
            int status;
            waitpid(job->pid, &status, WUNTRACED);  // Only wait for the specified job
                
            // Update the job status based on the result of waitpid
            if (WIFEXITED(status)) {
                updateJobStatus(job->pid, DONE);
            } else if (WIFSTOPPED(status)) {
                updateJobStatus(job->pid, STOPPED);
            }

            // Resume all other jobs after the foreground job completes
            for (int i = 0; i < jobCount; i++) {
                if (jobs[i].status == STOPPED && jobs[i].pid != job->pid) {
                    kill(jobs[i].pid, SIGCONT);  // Resume previously stopped jobs
                    jobs[i].status = RUNNING;    // Update their status to running
                }
            }

            if (job->status == DONE) { removeJob(job->pid); }

            foregroundPid = 0;  // Reset foreground PID
        } else {
            printf("icsh: %s: no such job\n", current[1]);
        }
    } else {
        printf("icsh: %s: invalid job specification\n", current[1]);
    }
    lastExitStatus = 0;
}

/* Handle the bg command */
void handleBg(char** current) {
    if (current[1][0] == '%') {
        int jobId = atoi(&current[1][1]);
        Job* job = findJobById(jobId);
        if (job != NULL && job->status == STOPPED) {
            printf("[%d]+ %s &\n", job->id, job->command);
            kill(job->pid, SIGCONT);
            updateJobStatus(job->pid, RUNNING);
        } else {
            printf("icsh: %s: no such job\n", current[1]);
        }
    } else {
        printf("icsh: %s: invalid job specification\n", current[1]);
    }
    lastExitStatus = 0;
}

/* Execute the external command */
void executeExternalCommand(char** current, int isBackground) {
    if (current[0] == NULL) { return; }

    // Check if the command is a pipe
    for (int i = 0; current[i] != NULL; i++) {
        if (strcmp(current[i], "|") == 0) {
            current[i] = NULL; // Split the commands at the pipe
            char** firstCmd = current;
            char** secondCmd = &current[i + 1];
            handlePipe(firstCmd, secondCmd);
            return;
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        lastExitStatus = 1;
    }
    /* Child process: Execute the external command */
    else if (pid == 0) {
        setpgid(0, 0); // Set the process group ID of the child process to its PID
        if (setupRedirection(current) == -1) {
            exit(EXIT_FAILURE);
        }
        execvp(current[0], current);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    /* Parent process: Wait for the child to finish */
    else {
        setpgid(pid, pid); // Set child process group
        if (isBackground) {
            char* commandWithAmpersand = tokensToStr(current, 0);
            strcat(commandWithAmpersand, "&");
            addJob(pid, commandWithAmpersand, RUNNING);
            printf("[%d] %d\n", jobCount, pid);
            free(commandWithAmpersand);
        } else {
            foregroundPid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);
            lastExitStatus = WIFEXITED(status) ? WEXITSTATUS(status) : 1;

            if (WIFSTOPPED(status)) {
                addJob(pid, tokensToStr(current, 0), STOPPED);
                printf("\n[%d]+ Stopped                  %s\n", jobCount, tokensToStr(current, 0));
            }

            foregroundPid = 0; // Reset the PID of the foreground process
        }    
    }
}

/* Handle the cd command */
void handleCd(char** current) {
    if (current[1] == NULL) {
        fprintf(stderr, "icsh: expected argument to \"cd\"\n");
        lastExitStatus = 1;
    } else {
        if (chdir(current[1]) != 0) {
            perror("icsh");
            lastExitStatus = 1;
        } else {
            lastExitStatus = 0;
            char cwd[MAX_CMD_BUFFER];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                snprintf(prompt, sizeof(prompt), "icsh%s $ ", cwd);
            } else {
                perror("getcwd");
            }
        }
    }
}

/* Handle the pipe functionality */
void handlePipe(char** firstCmd, char** secondCmd) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execvp(firstCmd[0], firstCmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execvp(secondCmd[0], secondCmd);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

/* Check if the command contains a pipe or redirection */
int hasSpecialToken(char** current) {
    for (int i = 0; current[i] != NULL; i++) {
        if (strcmp(current[i], "|") == 0 || strcmp(current[i], "<") == 0 || strcmp(current[i], ">") == 0) {
            return 1;
        }
    }
    return 0;
}

/* Handle the command based on the input tokens */
void handleCommand(char** current, char** previous) {
    if (!current || !current[0]) { return; } 
    int specialToken = hasSpecialToken(current); // Check "|" or "<" or ">" in the command
    if (specialToken) {
        executeExternalCommand(current, 0);
    } else if (strcmp(current[0], "cd") == 0) {
        handleCd(current);
    } else if (strcmp(current[0], "help") == 0 && current[1] == NULL) {
        printHelp();
        lastExitStatus = 0;
    } else if (strcmp(current[0], "echo") == 0 && current[1] != NULL && !strstr(tokensToStr(current, 0), "<") && !strstr(tokensToStr(current, 0), ">")) {
        handleEcho(current);
    } else if (strcmp(current[0], "!!") == 0 && current[1] == NULL) {
        if (previous && previous[0] && strcmp(previous[0], "!!") != 0) {
            handleCommand(copyTokens(previous), previous);
        } else {
            printf("No previous command\n");
        }
        lastExitStatus = 0;
    } else if (strcmp(current[0], "exit") == 0) {
        handleExit(current);
    } else if (strcmp(current[0], "jobs") == 0 && current[1] == NULL) {
        listJobs();
        lastExitStatus = 0;
    } else if (strcmp(current[0], "fg") == 0 && current[1] != NULL) {
        handleFg(current);
    } else if (strcmp(current[0], "bg") == 0 && current[1] != NULL) {
        handleBg(current);
    } else { // External command
        int isBackground = 0;
        for (int i = 0; current[i] != NULL; i++) {
            if (strcmp(current[i], "&") == 0) {
                isBackground = 1;
                current[i] = NULL; // Remove '&' from command
                break;
            }
        }
        executeExternalCommand(current, isBackground);
    }
}

/* Read and execute the commands from the given script file */
void readScripts(const char* fileName) {
    FILE* scriptFile = fopen(fileName, "r");
    if (!scriptFile) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_CMD_BUFFER];

    while (fgets(buffer, MAX_CMD_BUFFER, scriptFile)) {
        buffer[strcspn(buffer, "\r\n")] = 0;

        char** currCommandArgs = tokenizeInput(buffer); 
        handleCommand(currCommandArgs, lastCommandArgs);

        freeTokens(lastCommandArgs); 
        lastCommandArgs = copyTokens(currCommandArgs);
        freeTokens(currCommandArgs); 
    }

    fclose(scriptFile);
}

int main(int argc, char* argv[]) {
    setupSignalHandlers(); 

    if (argc > 1) { 
        readScripts(argv[1]);
    } else {
        char buffer[MAX_CMD_BUFFER];
        displayWelcomeMessage();

        while (1) {
            printf("%s", prompt);
            if (fgets(buffer, MAX_CMD_BUFFER, stdin) == NULL) {
                break;
            }
            buffer[strcspn(buffer, "\n")] = 0; 

            currCommandArgs = tokenizeInput(buffer); 
            handleCommand(currCommandArgs, lastCommandArgs);

            freeTokens(lastCommandArgs);
            lastCommandArgs = copyTokens(currCommandArgs);
            freeTokens(currCommandArgs);
        }
        freeTokens(lastCommandArgs);
    }

    return 0;
}