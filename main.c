#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

   ///////////////////////////////////////////////////////////
  //////////////// Built In Commands Section ////////////////
 ///////////////////////////////////////////////////////////

// Func declarations for built-in shell commands
int MicroShell_cd(char **args);
int MicroShell_curd(char **args);
int MicroShell_mdir(char **args);
int MicroShell_rdir(char **args);
int MicroShell_ip(char **args);
int MicroShell_pswdrand(char **args);
int MicroShell_help(char **args);
int MicroShell_exit(char **args);

// List of built-in commands
char *builtin_str[] = {
    "cd",
    "curd",
    "mdir",
    "rdir",
    "ip",
    "pswdrand",
    "help",
    "exit"
};

char *builtin_desc[] = {
    "cd: Change the current working directory \t\t\t\t Ex: cd /usr/bin/path",
    "curd: Lists the current directory and path \t\t\t\t Ex: curd",
    "mdir: Creates a directory in the current working path \t Ex: mdir /test",
    "rdir: Removes a directory in the current working path \t Ex: rdir /test",
    "ip: Tells you the ip of the current device \t\t\t\t Ex: ip",
    "pswdrand: A simple password generator \t\t\t\t\t: Ex pswdrand {LENGTH}",
    "help: Lists the commands and their uses \t\t\t\t Ex: help",
    "exit: Exits out of the terminal \t\t\t\t\t\t Ex: exit"
};

int (*builtin_func[])(char **) = {
    &MicroShell_cd,
    &MicroShell_curd,
    &MicroShell_mdir,
    &MicroShell_rdir,
    &MicroShell_ip,
    &MicroShell_pswdrand,
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

int MicroShell_mdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "MicroShell: expected argument\n");
        return 1;
    }

    mode_t mode = 0755;
    if (mkdir(args[1], mode) == -1) {
        perror("Error creating directory\n");
        return 1;
    } else {
        printf("Directory created\n");
    }
    return 1;
}

int MicroShell_rdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "MicroShell: expected argument\n");
        return 1;
    }

    if (rmdir(args[1]) == -1) {
        perror("Error removing directory\n");
    } else {
        printf("Directory removed.\n");
    }
    return 1;
}

int MicroShell_ip(char **args) {
    char hostbuffer[256];
    struct hostent *host_entry;

    // Get hostname
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        perror("gethostname");
        return 1;
    }

    // Get host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        perror("gethostbyname");
        return 1;
    }

    // Print all available IP addresses
    struct in_addr **addresses = (struct in_addr **)host_entry->h_addr_list;
    int i = 0;

    printf("IP addresses:\n");
    while(addresses[i] != NULL) {
        printf("%d: %s\n", i + 1, inet_ntoa(*addresses[i]));
        i++;
    }

    return 1;
}

int MicroShell_pswdrand(char **args) {
    // Checks if length is there
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: pswdrand LENGTH\n");
        return 1;
    }

    // Simple conversion using strtol
    char *endptr;
    long length = strtol(args[1], &endptr, 10);

    // Validate stuff
    if (*endptr != '\0' || endptr == args[1]) {
        fprintf(stderr, "Error: Invalid numeric value '%s'\n", args[1]);
        return 1;
    }
    if (length <= 0) {
        fprintf(stderr, "Error: Length must be positive\n");
        return 1;
    }

    // printf("Password length test: %d\n", length);

    // Memory allocated because why not...
    char *password = malloc(length + 1);

    char *digits = "0123456789";
    int digits_len = strlen(digits);

    char *lowers = "abcdefghijklmnopqrstuvwxyz";
    int lowers_len = strlen(lowers);

    char *uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int uppers_len = strlen(uppers);

    char *symbols = "~!@#$%^&*()-=_+{}[]|;:<,>.?/";
    int symbols_len = strlen(symbols);

    // Seed value making sure everything is different on every run
    srand(time(NULL) * getpid());

    for (int i = 0; i < length; i++)
    {
        int char_type = rand() % 4;


        if (char_type == 0) {
            password[i] = digits[rand() % digits_len];
        } else if (char_type == 1) {
            password[i] = lowers[rand() % lowers_len];
        } else if (char_type == 2) {
            password[i] = uppers[rand() % uppers_len];
        } else if (char_type == 3) {
            password[i] = symbols[rand() % symbols_len];
        }
    }
    password[length] = '\0';
    printf("Password: %s\n", password);

    free(password);
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