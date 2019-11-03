/***************************************************************************
  @author       OS Lab Group 3 (2019 - 20)
  @brief        SHa (SHell Application)
  @date         Sunday,  3 November 2019
****************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Parser/parser.h"
#include "Executioner/executioner.h"
#include "ShellBuiltins/shell-builtins.h"

/*
	main()
    @brief Main entry point.
    @param argc Argument count.
    @param argv Argument vector.
    @return status code
*/
int main(int argc , char **argv){
  // Run command loop.
  SHa_LOOP();
  
  return EXIT_SUCCESS;
}
