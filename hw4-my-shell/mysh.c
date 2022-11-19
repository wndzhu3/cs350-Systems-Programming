// Wendy Zhu
// wzhu62@emory.edu
/* THIS  CODE  WAS MY OWN WORK , IT WAS  WRITTEN  WITHOUT  CONSULTING  ANY
SOURCES  OUTSIDE  OF  THOSE  APPROVED  BY THE  INSTRUCTOR. Wendy Zhu  */

#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

char ** get_piped_tokens( const char * line) {
    char **tokens=NULL;
    char * line_copy;
    const char * delim = "|";
    char * cur_token;
    int num_tokens=0;

    tokens = (char**) malloc( sizeof(char*) );
    tokens[0] = NULL;

    if( line == NULL )
        return tokens;

    line_copy = strdup( line );
    cur_token = strtok( line_copy, delim );
    if( cur_token == NULL )
        return tokens;

    do {
        num_tokens++;
        tokens = ( char ** ) realloc( tokens, (num_tokens+1) * sizeof( char * ) );
        tokens[ num_tokens - 1 ] = strdup(cur_token);
        tokens[ num_tokens ] = NULL;
    } while( (cur_token = strtok( NULL, delim )) );
    free(line_copy);

    return tokens;
}

typedef struct {
  char * input_file;
  char * output_file;
  char ** command;
  bool append;
} Command;

typedef struct {
  int num_commands;
  Command * cmd_list;
  bool foreground;
} CommandSet;

typedef struct {
  char * prompt;
  int prompt_type;
} Options;

int error = 0;

Options parseArgs(int argc, char * argv[]) {
  Options opts;
  if (argc == 2) {
    if (strcmp(argv[1], "-") != 0) {
      opts.prompt_type = 2;
      opts.prompt = argv[1];
      return opts;
    } else {
      opts.prompt_type = 0;
      opts.prompt = "";
      return opts;
    }
  } else if (argc == 1) {
    opts.prompt_type = 1;
    opts.prompt = "mysh: ";
    return opts;
  }

  fprintf(stderr, "Error: Usage: mysh [prompt]\n");
  exit(1);
}

char ** removeOperators(char ** tokens, int op_loc) {
  int count = 0;

  for (; tokens[count] != NULL; count++) {

  }

  char ** cmd_tokens = malloc(sizeof(char *) * (count - 1));
  int offset = 0;

  int i = 0;
  while (tokens[i] != NULL) {
    if (!((offset != 2) && (i == op_loc || (i == op_loc + 1)))) {
      cmd_tokens[i - offset] = tokens[i];
    } else {
      offset++;
    }
    i++;
  }

  cmd_tokens[count - offset] = NULL;
  return cmd_tokens;
}

CommandSet parseCommands(char* cmd_line_args){
  CommandSet cmdset;
  char * background_char = strchr(cmd_line_args, '&');

  if (strcmp(&cmd_line_args[strlen(cmd_line_args) - 1], "&") != 0) {
    cmdset.foreground = true;
  } else {
    cmdset.foreground = false;
    cmd_line_args[strlen(cmd_line_args) - 1] = '\0';
  }

  if (!((strchr(cmd_line_args, '&') != NULL) && strcmp(background_char, "&"))) {
    char ** pipe_tokens = get_piped_tokens(cmd_line_args);
    int count = 0;

    while (pipe_tokens[count] != NULL) {
        count++;
    }

    cmdset.cmd_list = malloc(sizeof(Command) * count);

    int num_cmds = 0;
    while (pipe_tokens[num_cmds] != NULL) {
      Command new_cmd;
      char ** cmd_tokens = get_tokens(pipe_tokens[num_cmds]);

      new_cmd.command = cmd_tokens;
      new_cmd.input_file = malloc(sizeof(char) * 1024);
      new_cmd.output_file = malloc(sizeof(char) * 1024);
      new_cmd.append = false;

      int num_args = 0;
      while (new_cmd.command[num_args] != NULL) {
        if (strcmp(new_cmd.command[num_args], ">") == 0) {
          if (!(strcmp(new_cmd.output_file, "") != 0 || new_cmd.command[num_args + 1] == NULL)) {
            int fd = open(new_cmd.command[num_args + 1], O_CREAT | O_WRONLY | O_EXCL, S_IROTH | S_IWUSR | S_IRGRP | S_IRUSR);
            if (fd != -1) {
              new_cmd.output_file = new_cmd.command[num_args + 1];
              new_cmd.command = removeOperators(new_cmd.command, num_args);
              close(fd);
              if (num_args != 0) {
                num_args--;
              }
            } else {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", new_cmd.command[num_args + 1], strerror(errno));
              error = 1;
            }
          } else if (strcmp(new_cmd.output_file, "") != 0) {
            fprintf(stderr, "Error: Missing filename for output redirection.\n");
            error = 1;
          } else if (new_cmd.command[num_args + 1] == NULL) {
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
            error = 1;
          }
        }
        if (strcmp(new_cmd.command[num_args], "<") == 0 && new_cmd.command[num_args] != NULL) {
          if ((!(strcmp(new_cmd.input_file, "") != 0 || new_cmd.command[num_args + 1] == NULL))) {
            int fd = open(new_cmd.command[num_args + 1], O_RDONLY);
            if (fd != -1) {
              new_cmd.input_file = new_cmd.command[num_args + 1];
              new_cmd.command = removeOperators(new_cmd.command, num_args);
              close(fd);
              if (num_args < 0 || num_args > 0) {
                num_args -= 1;
              }
            } else {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", new_cmd.command[num_args + 1], strerror(errno));
              error = 1;
            }
          } else if (strcmp(new_cmd.input_file, "") != 0) {
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
            error = 1;
          } else if (new_cmd.command[num_args + 1] == NULL) {
            fprintf(stderr, "Error: Missing filename for input redirection.\n");
            error = 1;
          }
        }
        if (strcmp(new_cmd.command[num_args], ">>") == 0 && new_cmd.command[num_args] != NULL) {
          if (!(strcmp(new_cmd.output_file, "") != 0 || new_cmd.command[num_args + 1] == NULL)) {
            new_cmd.append = true;
            new_cmd.output_file = new_cmd.command[num_args + 1];
            new_cmd.command = removeOperators(new_cmd.command, num_args);
            if (num_args < 0 || num_args > 0) {
              num_args -= 1;
            }
          } else if (strcmp(new_cmd.output_file, "") != 0) {
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
            error = 1;
          } else if (new_cmd.command[num_args + 1] == NULL) {
            fprintf(stderr, "Error: Missing filename for output redirection.\n");
            error = 1;
          }
        }
        num_args++;
      }
      cmdset.num_commands = count;
      cmdset.cmd_list[num_cmds] = new_cmd;
      num_cmds++;
    }
  } else {
    fprintf(stderr, "Error: \"&\" must be last token on command line\n");
    error = 1;
  }
  return cmdset;
}

void checkForRedirectError(char * cmdargv) {
  int num_cmds = 0;
  char ** cmds = get_piped_tokens(cmdargv);

  while(cmds[num_cmds] != NULL){
      num_cmds++;
  }

  int input_num = 0;
  int output_num = 0;
  char ** tokens = get_tokens(cmdargv);
  int count = 0;

  if (strcmp(&cmdargv[strlen(cmdargv) - 1], "|") != 0) {
    int i = 0;
    while (tokens[i] != NULL) {
      char * t = tokens[i];
      if (!error) {
        if (strcmp(t, "<") == 0) {
          if (input_num >= 1) {
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
            error = 1;
          }
          input_num++;
        } else if (strcmp(t, ">") == 0 || strcmp(t, ">>") == 0) {
          if (output_num >= 1) {
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
            error = 1;
          }
          output_num++;
        } else if (strcmp(t, ">>") == 0) {
          if (output_num >= 1) {
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
            error = 1;
          }
          output_num++;
        } else if (strcmp(t, "|") == 0) {
          if (output_num >= 1) {
            output_num = 0;
            error = 1;
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
          } else if (input_num >= 2) {
            input_num = 0;
            error = 1;
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
          } else {
            input_num = 1;
            output_num = 0;
            count++;
          }
        }
      }
      i++;
    }
  } else {
    error = 1;
    fprintf(stderr, "Error: Ambiguous output redirection.\n");
  }
}

bool keepWaiting(int *foreground_processes, int num_cmds) {
  int sum = 0;
  int i = 0;
  while (i < num_cmds) {
    sum += foreground_processes[i];
    i++;
  }
  if (sum == (num_cmds * -1)) {
    return 0;
  } else {
    return 1;
  }
}

void setForegroundProcess(int * foreground_processes, int pid, int num_cmds) {
  int i = 0;
  while (i < num_cmds) {
    if (foreground_processes[i] == pid){
      foreground_processes[i] = -1;
    }
    i++;
  }
}

int main(int argc, char * argv[]) {
  Options opts = parseArgs(argc, argv);

  char ** cmd_tokens;
  bool cmds_executed = false;
  char * cmdargv;
  int * foreground_pids;
  char * fgets_return;
  int wpid;
  int status;
  int pid;

  while(1) {
    cmdargv = malloc(1024);
    printf("%s", opts.prompt);

    error = 0;

    do {
      fgets_return = fgets(cmdargv, 1024, stdin);
    } while (errno == EINTR && fgets_return == NULL);

    if (strcmp(cmdargv, "") == 0) {
      error = 1;
    }

    cmdargv[strlen(cmdargv) - 1] = '\0';

    if ((strcmp(cmdargv, "exit") == 0 || fgets_return == NULL) && errno != EINTR) {
      exit(0);
    }

    CommandSet cmdset;
    checkForRedirectError(cmdargv);

    if (!error) {
      cmdset = parseCommands(cmdargv);
      int i = 0;
      while (i < cmdset.num_commands) {
        if (cmdset.cmd_list[i].command[0] == NULL){
          error = 1;
          fprintf(stderr, "Error: Invalid null command.\n");
          break;
        }
        i++;
      }
    }

    if (!error) {
      int i = 0;
      while (i < cmdset.num_commands) {
        if (cmdset.num_commands > 1) {
          if (cmdset.num_commands > 1 && strcmp(cmdset.cmd_list[i].output_file, "") != 0 && i == 0) {
            error = 1;
            fprintf(stderr, "Error: Ambiguous output redirection.\n");
          } else if ((i == cmdset.num_commands - 1) && (strcmp(cmdset.cmd_list[i].input_file, "") != 0)) {
            error = 1;
            fprintf(stderr, "Error: Ambiguous input redirection.\n");
          } else if (i != cmdset.num_commands - 1 && i != 0) {
            if ((strcmp(cmdset.cmd_list[i].output_file, "") != 0)) {
              error = 1;
              fprintf(stderr, "Error: Ambiguous output redirection.\n");
            } else if (strcmp(cmdset.cmd_list[i].input_file, "") != 0) {
              error = 1;
              fprintf(stderr, "Error: Ambiguous input redirection.\n");
            }
          }
        }
        if (cmdset.cmd_list[i].command[0] == NULL) {
          fprintf(stderr, "Error: Invalid null command.\n");
          error = 1;
          break;
        }
        i++;
      }
      foreground_pids = malloc(sizeof(int) * cmdset.num_commands);
    }

    int pipefd[] = {-1, -1};
    int prevpipefd[] = {-1, -1};

    if (!error) {
      int cmd_index = 0;
      while (cmd_index < cmdset.num_commands) {
        if (cmd_index <= cmdset.num_commands - 2 && cmdset.num_commands >= 2) {
          pipe(pipefd);
        }

        pid = fork();
        if (pid != 0) {
          foreground_pids[cmd_index] = pid;

          if (cmd_index == cmdset.num_commands - 1 && cmdset.foreground) {
            while (keepWaiting(foreground_pids, cmdset.num_commands)) {
              do {
                wpid = wait(&status);
              } while (errno == EINTR && wpid < 0);
              setForegroundProcess(foreground_pids, wpid, cmdset.num_commands);
            }
          }
          if (cmdset.num_commands >= 2) {
            close(pipefd[1]);
            prevpipefd[1] = pipefd[1];
            close(prevpipefd[0]);
            prevpipefd[0] = pipefd[0];
          }
        } else {
          if (cmdset.num_commands >= 2 && cmd_index == 0) {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);
          } else if (cmdset.num_commands >= 2 && cmd_index == cmdset.num_commands - 1) {
            dup2(prevpipefd[0], 0);
            close(prevpipefd[0]);
          } else if (cmdset.num_commands >= 2) {
            dup2(pipefd[1], 1);
            close(pipefd[1]);
            close(pipefd[0]);
            dup2(prevpipefd[0], 0);
            close(prevpipefd[0]);
          }

          int fd;
          int co = strcmp(cmdset.cmd_list[cmd_index].output_file, "");

          if (cmdset.cmd_list[cmd_index].append && co != 0) {
            fd = open(cmdset.cmd_list[cmd_index].output_file, O_APPEND | O_CREAT | O_WRONLY, S_IROTH | S_IWUSR | S_IRUSR | S_IRGRP);
            if (fd != -1) {
              int dup_out = dup2(fd, 1);
              close(fd);
            } else {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", cmdset.cmd_list[cmd_index].output_file, strerror(errno));
              exit(0);
            }
          } else if (co != 0) {
            fd = open(cmdset.cmd_list[cmd_index].output_file, O_WRONLY | O_CREAT, S_IROTH | S_IWUSR | S_IRUSR | S_IRGRP);
            if (fd != -1) {
              int dup_out = dup2(fd, 1);
              close(fd);
            } else {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", cmdset.cmd_list[cmd_index].output_file, strerror(errno));
              exit(0);
            }

          }

          int ci = strcmp(cmdset.cmd_list[cmd_index].input_file, "");
          if (ci != 0) {
            fd = open(cmdset.cmd_list[cmd_index].input_file, O_RDONLY);
            if (fd != -1) {
              int dup_out = dup2(fd, 0);
              close(fd);
            } else {
              fprintf(stderr, "Error: open(\"%s\"): %s\n", cmdset.cmd_list[cmd_index].input_file, strerror(errno));
              exit(0);
            }
          }
          if (execvp(cmdset.cmd_list[cmd_index].command[0], cmdset.cmd_list[cmd_index].command) < 0) {
            perror("execvp()");
          }
          exit(-1);
        }
        cmd_index++;
      }
    }
  }
}
