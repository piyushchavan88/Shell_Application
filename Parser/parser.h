#define SHa_RL_BUFFSIZE 1024 //initial size of the buffer used to read line(RL)
/*
   Function Description : char * SHa_READLINE();	
   @brief Read a line of input from stdin.
   @return The line from stdin.
*/
char * SHa_READLINE(void){
	int buffsize = SHa_RL_BUFFSIZE;
	int position = 0; // variable used as a pointer for the buffer array
	
	//allocating the buffer
	char * buffer = malloc(sizeof(char) * buffsize);
	
	int ch;
	//variable for inputting characters from stdin

	//Buffer Allocation Error
 	if(!buffer){
	    fprintf(stderr, "SHa: RL_Buffer Allocation Error\n");
    	exit(EXIT_FAILURE);
  	}
	
	//inputting the characters
  	while(1){
    		//Read a character
    		ch = getchar();
		
		//checking if we have reached EOF or not
    		if(ch == EOF){
      			exit(EXIT_SUCCESS);
    		} 
    		else if(ch == '\n'){
    			//endline character is seen as end of the line
      			buffer[position] = '\0';
      			return buffer;
    		}
    		else{
      			buffer[position] = ch;
    		}
    		position++;

    		//If we have exceeded the buffer, reallocate the buffer.
    		if(position >= buffsize){
      			buffsize += SHa_RL_BUFFSIZE;
      			buffer = realloc(buffer, buffsize);
      		
      			//Buffer Allocation Error
      			if(!buffer) {
        			fprintf(stderr, "SHa: RL_Buffer Allocation Error\n");
        			exit(EXIT_FAILURE);
      			}
    		}
  	}
}

/* ----- Taking Input Finish -----*/

#define SHa_TOK_BUFFSIZE 64 //Initial size of the buffer where we split the input line and store the commands (tokens) to be passed to the executioner
#define SHa_TOK_DELIM " \t\r\n\a" //Delimiters according to which input string is to splitted
/*
	Function Description : char ** SHa_SPLITLINE(char * );
	@brief Split a line into tokens (very naively).
	@param line The line.
    @return Null-terminated array of tokens.
*/
char ** SHa_SPLITLINE(char * input){
	int buffsize = SHa_TOK_BUFFSIZE;
	int position = 0; //as a pointer to buffer
  
  	//allocating tokens buffer
  	char ** tokens = malloc(buffsize * sizeof(char*));
  	char *token, **tokens_backup;

	//Tokens Buffer Allocation Error 
  	if(!tokens){
    		fprintf(stderr, "SHa: TOK_Buffer Allocation Error\n");
    		exit(EXIT_FAILURE);
  	}
	
	//splitting the input string
  	token = strtok(input , SHa_TOK_DELIM);
  	
  	//storing splitted words in tokens buffer 
  	while(token != NULL){
    		tokens[position] = token;
    		position++;
		
		//If we have exceeded the buffer, reallocate the buffer.
    		if(position >= buffsize){
      			buffsize += SHa_TOK_BUFFSIZE;
      			tokens_backup = tokens;
      			tokens = realloc(tokens, buffsize * sizeof(char*));
      		
      			//Tokens Buffer Allocation Error 
      			if(!tokens){
				free(tokens_backup);
        			fprintf(stderr, "SHa: TOK_Buffer Allocation Error\n");
        			exit(EXIT_FAILURE);
      			}
    		}

    		token = strtok(NULL , SHa_TOK_DELIM);
  	}
  	tokens[position] = NULL;
  	return tokens;
}
