# Shell Programming Application
A shell Program to implement Grammer in C language in shell.I and shell.y to make our parser interpret the command lines and provide our executor with the correct information and performs task given to it.
       
       
## Project Description
The shell is a program that interacts with the user through a terminal or takes the input from a file and executes a sequence of commands that are passed to the Operating System. 

I am sure you may have come across using a shell. You have used command prompt in windows, or the popular bash in Linux. But how do they work? Lets see if we can break it down.

The upside down vision of the shell gives information about the three main things in its lifetime

* Start :- reading and executing the configuration files, changing its behavioural aspects
* Interpret:- After that, the shell reads commands from stdin (files or interactive inputs) and executes them.
* Terminate: After execution of commands, the shell executes any shutdown commands, frees up any memory, and terminates.
Now the shell consists of number of rules which we take as granted. And by these rules I meant think of it as a computer working on your high school algebra.

E.g: `(1+2 % 4 x (1 + 1))`

It would have to work out the BDMAS (brackets first, then divide, multiply, addition and subtraction respectively) rule here. Think of the possibilities of the user input first. You first need to make sure that the input is valid and error-free (You don't want your device to do unexpected things when the user enter unexpected input). You would then need to break down each individual tokens into an ordered list, check if there are any errors in the syntax and then try and parse them into an execution tree. 
       
## Brief Summary
           
Start of the Program

We'll just highlight the main concepts. We start at the entry point of the program. 

```cpp
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
```

### Looping
Then there is a main loop which would frequently scan for commands, parse it into token, check the grammar and syntax and either produce an error or an output the resulting command. A simple way to handle commands is with three steps:
* Read: Read the command from standard input.
* Parse: Separate the command string into a program and arguments.
* Execute: Run the parsed command.

If we translate it into the program then it will look as

```cpp
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}
```

### How it will read lines 
We will implement lsh_read_line() so that our program will be able to read the commands

```cpp
#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}
```

## The Lexical Analyzer
The lexical analysis is probably the most simplest, yet an important part of the shell. All it does is simply break the input string from the user to a series of tokens. Now tokens could either be a character or a series of characters. It also groups characters that are inside a single or double quotations.
       
### Parsing the Line
This topic is probably the most actively discussed topic in compiler theory and designs. 
we will simply use whitespace to separate arguments from each other. So the command echo "this message" would not call echo with a single argument this message, but rather it would call echo with two arguments: "this and message".

With those simplifications, all we need to do is “tokenize” the string using whitespace as delimiters.

```cpp
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
```

At the start of the function, we begin tokenizing by calling strtok. It returns a pointer to the first token. What strok() actually does is return pointers to within the string you give it, and place \0 bytes at the end of each token. We store each pointer in an array(buffer) of character pointers. 

At last, we reallocate the array of pointers if necessary. The process repeats until no token is returned by strlok, at which point we null terminate the list of tokens. 

## Execution
           
### How shell Processes start working
There are two processes to start the first is by init. when a Unix computer boots, its kernel is loaded. Once it is loaded and initialized, the kernel starts only one process, which is called Init. This process runs for the entire length of time that the computer is on, and it manages loading up the rest of the processes that you need for your computer to be useful.

Since most programs aren’t Init, that leaves only one practical way for processes to get started: the fork() system call. When this function is called, the operating system makes a duplicate of the process and starts them both running. The original process is called the “parent”, and the new one is called the “child”. Fork() returns 0 to the child process, and it returns to the parent the process ID number (PID) of its child. In essence, this means that the only way for new processes is to start is by an existing one duplicating itself.

Typically, when you want to run a new process, you don’t just want another copy of the same program – you want to run a different program. That’s what the exec() system call is all about. It replaces the current running program with an entirely new one. This means that when you call exec, the operating system stops your process, loads up the new program, and starts that one in its place. A process never returns from an exec() call (unless there’s an error).

With these two system calls, we have the building blocks for how most programs are run on Unix. First, an existing process forks itself into two separate ones. Then, the child uses exec() to replace itself with a new program. The parent process can continue doing other things, and it can even keep tabs on its children, using the system call wait().

The code to launch program is
```cpp
int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}
```

This function takes the list of arguments that we created earlier. Then, it forks the process, and saves the return value. Once fork() returns, we actually have two processes running concurrently. The child process will take the first if condition (where pid == 0).

In the child process, we want to run the command given by the user. So, we use one of the many variants of the exec system call, execvp. The different variants of exec do slightly different things. Some take a variable number of string arguments. Others take a list of strings. Still others let you specify the environment that the process runs with. This particular variant expects a program name and an array (also called a vector, hence the ‘v’) of string arguments (the first one has to be the program name). The ‘p’ means that instead of providing the full file path of the program to run, we’re going to give its name, and let the operating system search for the program in the path.

If the exec command returns -1 (or actually, if it returns at all), we know there was an error. So, we use perror to print the system’s error message, along with our program name, so users know where the error came from. Then, we exit so that the shell can keep running.

The second condition (pid < 0) checks whether fork() had an error. If so, we print it and keep going – there’s no handling that error beyond telling the user and letting them decide if they need to quit.

The third condition means that fork() executed successfully. The parent process will land here. We know that the child is going to execute the process, so the parent needs to wait for the command to finish running. We use waitpid() to wait for the process’s state to change. Unfortunately, waitpid() has a lot of options (like exec()). Processes can change state in lots of ways, and not all of them mean that the process has ended. A process can either exit (normally, or with an error code), or it can be killed by a signal. So, we use the macros provided with waitpid() to wait until either the processes are exited or killed. Then, the function finally returns a 1, as a signal to the calling function that we should prompt for input again.

## Buitin’s of Shell
The lsh_loop() function calls lsh_execute(), but above, we titled our function lsh_launch().

The reason is actually pretty simple. If you want to change directory, you need to use the function chdir(). The thing is, the current directory is a property of a process. So, if you wrote a program called cd that changed directory, it would just change its own current directory, and then terminate. Its parent process’s current directory would be unchanged. Instead, the shell process itself needs to execute chdir(), so that its own current directory is updated. Then, when it launches child processes, they will inherit that directory too.

Similarly, if there was a program named exit, it wouldn’t be able to exit the shell that called it. That command also needs to be built into the shell. Also, most shells are configured by running configuration scripts, like `~/.bashrc`. Those scripts use commands that change the operation of the shell. These commands could only change the shell’s operation if they were implemented within the shell itself.

So, it makes sense that we need to add some commands to the shell itself. The ones I added to my shell are cd, exit, and help. Here are their function implementations below:

```cpp
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0;
}
```

There are three parts to this code. The first part contains forward declarations of my functions. A forward declaration is when you declare (but don’t define) something, so that you can use its name before you define it. The reason I do this is because lsh_help() uses the array of builtins, and the arrays contain lsh_help(). The cleanest way to break this dependency cycle is by forward declaration.

The next part is an array of builtin command names, followed by an array of their corresponding functions. This is so that, in the future, builtin commands can be added simply by modifying these arrays, rather than editing a large “switch” statement somewhere in the code. If you’re confused by the declaration of builtin_func, that’s OK! I am too. It’s an array of function pointers (that take array of strings and return an int). Any declaration involving function pointers in C can get really complicated. I still look up how function pointers are declared myself!

Finally, I implement each function. The lsh_cd() function first checks that its second argument exists, and prints an error message if it doesn’t. Then, it calls chdir(), checks for errors, and returns. The help function prints a nice message and the names of all the builtins. And the exit function returns 0, as a signal for the command loop to terminate.

## Bringing all things Together

Make sure to only include one implementation of lsh_read_line(). You’ll need to include the following headers at the top. I’ve added notes so that you know where each function comes from.
* #include <sys/wait.h>
    * waitpid() and associated macros
* #include <unistd.h>
    * chdir()
    * fork()
    * exec()
    * pid_t
* #include <stdlib.h>
    * malloc()
    * realloc()
    * free()
    * exit()
    * execvp()
    * EXIT_SUCCESS, EXIT_FAILURE
* #include <stdio.h>
    * fprintf()
    * printf()
    * stderr
    * getchar()
    * perror()
* #include <string.h>
    * strcmp()
    * strtok()

Once you have the code and headers, it should be as simple as running gcc -o main main.c to compile it, and then ./main to run it.

Alternatively, you can get the code from GitHub. That link goes straight to the current revision of the code at the time of this writing– I may choose to update it and add new features someday in the future. If I do, I’ll try my best to update this article with the details and implementation ideas.

## Output for Some Shell Codes

Code 1:  `ls- l` 

Output :
```
tarun@krishna:~/Desktop$ bash final.sh
total 1072
-rw-rw-r--  1 tarun tarun      27 Nov  3 16:44  final.sh
-rw-rw-r--  1 tarun tarun     578 Nov  3 16:30  jai
-rw-rw-r--  1 tarun tarun 1051391 Jul 10  2018  krishna-syamasundara_1.jpg
-rw-rw-r--  1 tarun tarun    3464 Nov  3 16:36  NewScript
drwxr-xr-x 31 tarun tarun    4096 Nov  2 14:50 'Previous desktop'
-rw-rw-r--  1 tarun tarun     117 Nov  3 16:30  program.sh
-rw-rw-r--  1 tarun tarun   19119 Nov  2 17:35  readme.docx
```

Code 2: Hello Program In shell
```sh
#!/Desktop/NewScript
echo "what is your name?"
read name
echo "How do you do $name ?"
read remark
echo "I am $remark too."
```   
Output:
```
tarun@krishna:~/Desktop$ bash program.sh 
what is your name?
Tarun
How do you do Tarun ?
Good, nice
I am Good, nice too.
This is the way you run using this NewScript made by newScript
Well, I hope that was informative
Tarun
```
