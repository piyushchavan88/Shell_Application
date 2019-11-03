
int SHa_cd(char **args);
int SHa_help(char **args);
int SHa_exit(char **args);
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (builtin_func[]) (char *) = {
  &SHa_cd,
  &SHa_help,
  &SHa_exit
};

int SHa_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int SHa_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "SHa: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("SHa");
    }
  }
  return 1;
}

int SHa_help(char **args)
{
  int i;
  printf("Stephen Brennan'SHa\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < SHa_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int SHa_exit(char **args)
{
  return 0;
}
