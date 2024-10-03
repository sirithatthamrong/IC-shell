# ICSH (IC-shell)
This project was completed as a part of the ICCS271: Principles of Computer Systems and Architecture course by Kanladaporn Sirithatthamrong.

## Overview
In this project, I will implement a simple shell for Linux. This shell will be called “icsh” or “IC shell”. The functionality of this shell will be similar to other popular Linux shells such as bash, csh, zsh, but with a subset of features. Basically, icsh should have the following functionality:
- Interactive and batch mode
- Support built-in some commands 
- Allow the user to execute one or more programs from executable files as either background or foreground jobs
- Provide job-control, including a job list and tools for changing the foreground/background status of currently running jobs and job suspension/continuation/termination.
- Allow for input and output redirection to/from files.

## Interactive command-line interpreter
- `echo <text>`: Prints a given text (until EOL) back to the console.
- `!!`: Repeat the last command given to the shell.
- `exit <num>`: Exits the shell with a given exit code.

## Script mode
- Be able to read commands from the given file.

## Running an external program in the foreground
- When the given command doesn't match with any built-in command, it is assumed to be an external command.
- The shell must spawn a new process, execute and wait for the command to complete and resume control of the terminal.

## Signal Handler
- `Control-Z` (SIGTSTP): Suspend the processes in the current foreground job.
- `Control-C` (SIGINT): Kill the process in the current foreground job.

## I/O redirection
- `> <file>`: Output redirection.
- `< <file>`: Input redirection.

## Background jobs and job control
- `<command> &`: Run a command in the background 
- `jobs`: List all the current jobs.
- `fg %<job_id>`: Brings the job identified by <job_id> into the foreground.
- `bg %<job_id>`: Execute the suspended job identified by <job_id> in the background.

## Extra features
- `help`: Print the help menu.
