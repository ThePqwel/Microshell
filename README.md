MicroShell ★ – Custom Linux Interpreter
A lightweight, custom command-line interpreter written in C. This project demonstrates the fundamentals of Linux system programming, including process management, file I/O, and command execution.

How to Run
Follow these steps to compile and launch the shell on a Linux system.

1. Prerequisites
Ensure you have a C compiler (like gcc) installed.

gcc --version

2. Compilation
Open your terminal in the directory containing main.c and run:

gcc main.c -o myshell

3. Launching the Shell
To start the program, execute:

./myshell
You should now see the prompt: moj_shell> 

Key Features

★ External Execution: Runs standard system commands (e.g., ls, pwd) using fork() and execvp().

★ Redirection: Supports > operator using dup2() to save output to files.

★ Custom Tools: Includes built-in mycp (buffered file copy) and mytouch.

★ Built-ins: Native support for cd, help, and exit.
