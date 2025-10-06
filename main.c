#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <libgen.h>

////////////////////////////////////////////////////////////////////
//////////////////      Built In Commands Section
///////////////////////////////////////////////////////////////////


// Func declarations for built-in shell commands
int MicroShell_cd(char **args);
int MicroShell_curd(char **args);
int MicroShell_help(char **args);
int MicroShell_exit(char **args);

// List of built-in commands
char *builtin_str[] = {
    "cd",
    "curd",
    "help",
    "exit"
};

char *builtin_desc[] = {
    "cd: Change the current working directory \t\t Ex: cd /usr/bin/path",
    "curd: Lists the current directory and path \t\t Ex: curd",
    "help: Lists the commands and their uses \t\t Ex: help",
    "exit: Exits out of the terminal \t\t\t\t Ex: exit"
};

int (*builtin_func[])(char **) = {
    &MicroShell_cd,
    &MicroShell_curd,
    &MicroShell_help,
    &MicroShell_exit
};

int MicroShell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Built-in function implementations
int MicroShell_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME")) != 0) {
            perror("MicroShell");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("MicroShell");
        }
    }

    char cwd[1024];
    char cwd_copy[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strncpy(cwd_copy, cwd, sizeof(cwd_copy));
        char *folder = basename(cwd_copy);

        printf("[%s] %s\n", folder, cwd);
    } else {
        perror("MicroShell");
    }

    return 1;
}

int MicroShell_curd(char **args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
    } else {
        perror("MicroShell");
    }
    return 1;
}


int MicroShell_help(char **args) {
    int i;
    printf("Microwaves Shell\n");
    printf("Built-in commands:\n");

    for (i = 0; i < MicroShell_num_builtins(); i++) {
        printf("\t%s\n", builtin_desc[i]);
    }

    return 1;
}

int MicroShell_exit(char **args) {
    return 0;
}

#define MICROSHELL_BUFFER_SIZE 1024
char *MicroShell_read_line() {
    int bufsize = MICROSHELL_BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "MicroShell: allocation error\n");
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

        // If exceeded buffer, reallocate
        if (position >= bufsize) {
            bufsize += MICROSHELL_BUFFER_SIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "MicroShell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define MICROSHELL_TOK_BUFSIZE 64
#define MICROSHELL_TOK_DELIM " \t\r\n\a"
char **MicroShell_split_line(char *line) {
    int bufsize = MICROSHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "MicroShell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MICROSHELL_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MICROSHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "MicroShell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MICROSHELL_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int MicroShell_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("MicroShell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("MicroShell");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int MicroShell_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // Empty command
        return 1;
    }

    for (i = 0; i < MicroShell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return(*builtin_func[i])(args);
        }
    }

    return (MicroShell_launch(args));
}

void MicroShell_loop(void) {
    char *line;
    char cwd[1024];
    char cwd_copy[1024];
    char usrname[100];
    char **args;
    int status;

    do {
        getcwd(cwd, sizeof(cwd));
        getlogin_r(usrname, sizeof(usrname));
        strncpy(cwd_copy, cwd, sizeof(cwd_copy));
        char *folder = basename(cwd_copy);
        printf("[%s %s]> ", usrname, folder);
        line = MicroShell_read_line();
        args = MicroShell_split_line(line);
        status = MicroShell_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) {
    // Load config files

    // Performs the command loop
    MicroShell_loop();

    // Perform shutdown/cleanup

    return EXIT_SUCCESS;
}
