/*
NAME : Saravanakumar
DATE : 3/08/2024
DESCRIPTION : P1 :- Minishell
SAMPLE I/P AND O/P:
Description:

Implement a minimalistic shell, mini-shell(msh) as part of the Linux Internal module.

Objective:
The objective is to understand and use the system calls w.r.t process creation, signal handling, process synchronization, exit status, text parsing etc..

Requirement details:

1.Provide a prompt for the user to enter commands
1. Display the default prompt as msh>
2. Prompt should be customizable using environmental variable PS1
 • To change the prompt user will do PS1=NEW_PROMPT
 • Make sure that you do not allow whitespaces between = i.e., do not allow PS1 = NEW_PROMPT
• In the above case, it should be treated like a normal command

2. Execute the command entered by the user
1. User will enter a command to execute
2. If it is an external command
 • Create a child process and execute the command
• Parent should wait for the child to complete
 • Only on completion, msh prompt should be displayed
 • If user entering without a command should show the prompt again

3 Special Variables:
1. Exit status of the last command (echo $?)
 • After executing a command the exit status should be available
 • echo $? should print the exit status of the last command executed
2. PID of msh (echo $$)
• echo $$: should print msh's PID
3. Shell name (echo $SHELL)
• echo $SHELL: should print msh executable path

 4 Signal handling
Provide shortcuts to send signals to running program
1. Ctrl-C (Send SIGINT)
     On pressing Ctrl-C
• If a programming is running in foreground, send SIGINT to the program (child process)
• If no foreground program exists, re-display the msh prompt 2. Ctrl+z (Send SIGSTP)
2. Ctrl-z (Send SIGTSTP)
      On pressing Ctrl+z
• The program running in foreground, should stop the program and parent will display pid of child 2.5

5.Built-in commands
1. exit exit: This command will terminate the msh program
2. cd cd: Change directory
3. pwd: show the current working directory

6 Background Process / Job control
1. Allow a program to run in background To run a program in background use ampersand (&) after the command. For eg: sleep 50 &
2. Implement fg, bg and jobs commands
• bg will will move a stopped process to background sleep 10 & is equallent to sleep 10 then ctrl + z and bg
After this the msh prompt should be displayed indicating it is readyto accept further commands. After a bg process ends, cleanup the process using wait. 
NOTE: You may have to use SIGCHLD signal handler for this 
On termination of bg process, display its exit status.User should be able to run any number of background processes. 
• fg will bring a background process to foreground. 
  Only fg bring last background process, or fg will bring given pid to foreground
       • jobs will print all background process details.
7 Pipe functionality
1. Allow multiple processes communication by using pipes.
2. Create pipes and childs dynamically as per pipes passed from command-line
Eg: ls | wc, ls -l /dev | grep tty | wc -l
*/

#include "minishell.h"

int exit_val;
int job_count = 0;  // Initialize job_count to 0
Job jobs[MAX_JOBS]; // Define the jobs array
pid_t foreground_pid = -1;  // Initialize foreground_pid to -1

int get_command(char * input, char * temp) {
  int i;
  for (i = 0; input[i] != ' ' && input[i] != '\0'; i++) {
    temp[i] = input[i];
  }
  temp[i] = '\0';
  return i;
}
int special_variable(char * input) {
  // input = echo $$, $?, $SHELL, String
  char temp[10];
  int i = get_command(input, temp);
  if (strcmp(temp, "echo") == 0) {
    if (strcmp(input + i + 1, "$$") == 0) {
      printf("%d\n", getpid());
    } else if (strcmp(input + i + 1, "$?") == 0) {
      printf("%d\n", exit_val);
    } else if (strcmp(input + i + 1, "$SHELL") == 0) {
      printf("%s/minnishell\n",
        get_current_dir_name()); // current working directory
    } else {
      printf("%s", input + i + 1);
    }
    return SUCCESS;
  }
  return FAILURE;
}
void signal_handler(int signum) {
    if (foreground_pid > 0) {
        if (signum == SIGINT) { // Handle Ctrl+C
            printf("\nInterrupting process (PID: %d)...\n", foreground_pid);
            kill(foreground_pid, SIGKILL);
        } else if (signum == SIGTSTP) { // Handle Ctrl+Z
            printf("\nStopping process (PID: %d)...\n", foreground_pid);
            kill(foreground_pid, SIGTSTP);
            
            // Add the process to the jobs list and mark it as stopped
            add_job(foreground_pid, "Stopped process");
            jobs[job_count - 1].stopped = 1;
        }
        foreground_pid = -1;
    }
}


int main() {
  char input[50], prompt[50] = "minshell$";
// Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
  while (1) {
    printf(ANSI_COLOUR_YELLOW"[%s]$ "ANSI_COLOUR_CYAN ,prompt);
    fgets(input, 50, stdin); // input = PS1=NEW_PROMPT\n
    // Remove the newline character from the input
    input[strlen(input) - 1] = '\0';

    if (strncmp(input, "PS1=", 4) == 0) {
      strcpy(prompt, input + 4);
    } else if (special_variable(input) == SUCCESS) {
      continue;
    } else if (check_command_type(input) == EXTERNAL) {
      execute_external_command(input);
    } else if (check_command_type(input) == INTERNAL) {
      execute_internal_command(input);
    }
  }
}
