#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define string_BUFSIZE 1024
#define token_BUFSIZE 64
#define token_DELIMETERS " \t\r\n\f"

int myCD(char **args);
int myHELP(char **args);
int myEXIT(char **args);
int launchCode(char **args);
int executeCode(char **args);
char *readLine(void);
char **splitLine(char *line);
void startShell(void);

char *builtin_str[] = {"cd","help","exit"};
int (*builtin_func[]) (char **) = { &myCD,&myHELP,&myEXIT };
int totalnum_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int main()
{
  startShell();
  return EXIT_SUCCESS;
}

int myCD(char **args)
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

int myHELP(char **args)
{
  int i;
  printf("Welcome to Shell :- \n");
 
  printf("The following are built-in Functions to be used :\n");

  for (i = 0; i < totalnum_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  printf("Type program names and arguments, and hit enter to run them.\n");
  printf("Else, Use the man command for information on other programs.\n");
  return 1;
}

int myEXIT(char **args)
{
  return 0;
}
int launchCode(char **args)
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

int executeCode(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < totalnum_builtins(); i++)
  {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return launchCode(args);
}
char *readLine(void)
{
  int bufsize = string_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
   
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= bufsize) {
      bufsize += string_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **splitLine(char *line)
{
  int bufsize = token_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, token_DELIMETERS);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += token_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, token_DELIMETERS);
  }
  tokens[position] = NULL;
  return tokens;
}

void startShell(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("Shell/commandPrompt> ");
    line = readLine();
    args = splitLine(line);
    status = executeCode(args);  
    free(line);
    free(args);
  } while (status);
}
