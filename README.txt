Benjamin Knight
https://github.com/ben8622/Linux-Shell-Clone

Things to be updated:
-Better implementation of OOP for reusability
-implement !n function
-better error management
-allow more history elements
-allow more PID elements

DESC:
~~~~~~~~~~~~~~~~~
This project was to understand the use and implemention of the exec() and fork()
system calls. The linux shell, in a very abstracted description, will read arguments
put into the command line and determine if:

1.) Is it a valid command?
2.) Is the function being requested a built-in function?
2.) Is the function being requested an external function?

-If the function is NOT valid the program will ignore the input (including empty input)

-If the function is valid and is a built-in, the program simply calls that function.

-If the function is valid and an external executable, the program will fork() a
new process then use execvp() on said process to replace the current program that the 
process is running with the specified function. Process continues until the function 
has finished and is then reaped.

After this the programm will lopp back read to accept user input again. The program can 
be terminated with "exit" or "quit" being called.
~~~~~~~~~~~~~~~~~~~~

