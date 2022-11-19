## Homework 4: My Shell (Command Line Interpreter)

This assignment implements `mysh`, which runs a command line interpreter. I apply C/UNIX process management system calls, manipulate and redirect a program's standard input, output, and error streams, and do inter-process communication operations.

`mysh` invokes a command line interpreter that supports command execution and input/output redirection. `mysh` takes a single optional command line argument that specifies the prompt string. If no arguments are given, the prompt defaults to “`mysh: `”.

### Usage

`mysh [prompt]`

**Operators**

`mysh` supports the following command line operators:

- `&`: Place commands into the background: after invoking the specified commands, mysh immediately prints its prompt and waits for another command line. `&` may only be specified as the final command line argument.
- `<`: Redirect the current command’s standard input stream from the file named immediately after the `<` operator.
- `>`: Redirect the current command’s standard output stream to the file named immediately after the `>` operator.
- `>>`: Redirect the current command’s standard output stream to the file named immediately after the `>>` operator. Append to the file if it already exists.
- `|`: Redirect the current command’s standard output stream to the standard input stream of the succeeding command. There may be *any* number of pipe-connected processes.
