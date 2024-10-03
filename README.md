# ICSH (IC-shell)
This project was completed as a part of the ICCS271: Principles of Computer Systems and Architecture course by Kanladaporn Sirithatthamrong.

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
