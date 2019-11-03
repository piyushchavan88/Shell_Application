
/*
	FUNCTION 1 :  SHa_EXECUTE()
	DESCRIPTION :
 		This is function of execution for the shell program.
 		This function is to execute the shell(built-in or launch the shell program).
 		args is NUll terminated list of arguments passed to the function.
 		It returns 1 if the shell has to continue reading .
 		It returns 0 if termination is required .
 */
int SHa_EXECUTE(char **args){
  	if (args[0] == NULL) {
    		return 1;
 	}
	int i;
  	for(i = 0; i < SHa_num_builtins(); i++) {
    	if (strcmp(args[0], builtin_str[i])==0) {
      		return(*builtin_func[i])(args);
    	}
  	}
  	return SHa_LAUNCH(args);
}

/*
	FUNCTION 2: SHa_LAUNCH()
	DESCRIPTION :
 		This is function to launch the shell program and then wait for it to terminate.
 		args is NUll terminated list of arguments passed to the function.
 		It always returns 1 so that the execution can continue .
*/
int SHa_LAUNCH(char **args){
  	pid_t pid;
  	int status;
  	pid = fork();
  	if(pid == 0){
    	if(execvp(args[0], args) == -1){
      		perror("SHa");
    	}
    	exit(EXIT_FAILURE);
  	} 
	else if(pid < 0){
    	perror("SHa");
  	} 
	else{
    	do{
      		waitpid(pid, &status, WUNTRACED);
    	}while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
  	return 1;
}

/*
	FUNCTION 3: SHa_LOOP()
 	DESCRIPTION :
 		This is function to take input from the parser and then execute it.
 		It does not return anything and only performs the execution .
*/
void SHa_LOOP(void){
  	char *line;
  	char **args;
  	int status;
  	do{
    	printf("> ");
    	line = SHa_READLINE();
    	args = SHa_SPLITLINE(line);
    	status = SHa_EXECUTE(args);
    	free(line);
    	free(args);
  	} 
	while(status);
}

