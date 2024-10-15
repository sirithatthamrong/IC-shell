# ICSH (IC-shell)
This project was completed as a part of the ICCS271: Principles of Computer Systems and Architecture course by Kanladaporn Sirithatthamrong.

## Overview
In this project, I will implement a simple shell for Linux. This shell will be called “icsh” or “IC shell”. The functionality of this shell will be similar to other popular Linux shells such as bash, csh, zsh, but with a subset of features. Basically, icsh should have the following functionality:
- Interactive and batch mode
- Support built-in some commands 
- Allow the user to execute one or more programs from executable files as either background or foreground jobs
- Provide job-control, including a job list and tools for changing the foreground/background status of currently running jobs and job suspension/continuation/termination.
- Allow for input and output redirection to/from files.

## Commands Implemented

The following commands are available in the IC shell:

| Command                         | Description                                                |
|---------------------------------|------------------------------------------------------------|
| `help`                          | Display the help menu                                      |
| `cd <dir>`                      | Change the current directory                               |
| `echo <text>`                   | Print the given text to the console                        |
| `!!`                            | Repeat the last command                                    |
| `exit <num>`                    | Exit the shell with a given exit code                      |
| `./icsh <script_file>`          | Execute a script file                                      |
| `> <file>`                      | Redirect output to a file                                  |
| `< <file>`                      | Redirect input from a file                                 |
| `<command> &`                   | Run a command in the background                            |
| `jobs`                          | List all background jobs                                   |
| `fg %<job_id>`                  | Bring a background job to the foreground                   |
| `bg %<job_id>`                  | Resume a stopped background job                            |
| `<command1> \| <command2>`       | Pipe the output of command1 to command2                   |

