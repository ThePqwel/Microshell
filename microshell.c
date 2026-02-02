#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64
#define BUFFER_SIZE 4096

#define COL_RESET "\033[0m"
#define COL_BORDO "\033[38;2;108;8;32m" 
#define COL_PINK  "\033[38;2;242;174;188m"
#define COL_LIGHT "\033[38;2;242;220;219m"
#define COL_BLUE  "\033[38;2;90;134;203m"
#define COL_DARK_BLUE "\033[38;2;61;93;145m"

void handle_sigint(int sig) {
    printf("\n"); \
}

void print_error(const char *msg) {
    fprintf(stderr, "%sError: %s%s\n", COL_BORDO, msg, COL_RESET);
}

void print_prompt() {
    char cwd[1024];
    char *user = getenv("USER");
    
    if (user == NULL) {
        user = "unknown";
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s[%s%s%s] %s%s%s %s$%s ", 
            COL_LIGHT,
            COL_DARK_BLUE, user, COL_LIGHT,
            COL_BLUE, cwd, COL_RESET,
            COL_PINK, COL_RESET
        );
    } else {
        perror("getcwd error");
    }
    fflush(stdout);
}

void print_intro() {
    printf("%s", COL_PINK);
    printf("        .\n");
    printf("       ,O,\n");
    printf("      ,OOO,\n");
    printf("'oooooOOOOOooooo'\n");
    printf("  `OOOOOOOOOOO`\n");
    printf("    `OOOOOOO`\n");
    printf("    OOOO'OOOO\n");
    printf("   OOO'   'OOO\n");
    printf("  O'         'O\n");
    printf("%s\n", COL_RESET);
}

int run_help(char **args) {
    printf("%s=== ★ Microshell ===%s\n", COL_PINK, COL_RESET);
    printf("Autor: s500825\n");
    printf("%sWbudowane polecenia:%s\n", COL_LIGHT, COL_RESET);
    printf("  cd <path>        - zmiana katalogu\n");
    printf("  exit             - wyjscie z powloki\n");
    printf("  help             - wyswietla te pomoc\n");
    printf("  mycp <src> <dst> - kopiowanie pliku\n");
    printf("  mytouch <file>   - tworzenie pliku\n");
    return 1;

}
int run_exit(char **args) {
    return 0;
}

int run_cd(char **args) {
    int result;

    if (args[1] == NULL) {
        print_error("Oczekiwano argumentu dla \"cd\"");
    } else {
        result = chdir(args[1]);
        if (result != 0) {
            perror("cd error");
        }
    }
    return 1;
}

int run_mycp(char **args) {
    int src_fd, dst_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];

    if (args[1] == NULL || args[2] == NULL) {
        print_error("Uzycie: mycp <zrodlo> <cel>");
        return 1;
    }

    src_fd = open(args[1], O_RDONLY);
    if (src_fd < 0) {
        perror("mycp (open source)");
        return 1;
    }

    dst_fd = open(args[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0) {
        perror("mycp (create dest)");
        close(src_fd);
        return 1;
    }

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("mycp (write)");
            close(src_fd);
            close(dst_fd);

            return 1;
        }
    }

    if (bytes_read < 0) {
        perror("mycp (read)");
    }

    close(src_fd);
    close(dst_fd);
    return 1;
}

int run_mytouch(char **args) {
    int fd;

    if (args[1] == NULL) {
        print_error("Uzycie: mytouch <nazwa_pliku>");
        return 1;
    }

    fd = open(args[1], O_WRONLY | O_CREAT | O_NOCTTY | O_NONBLOCK, 0666);
    if (fd < 0) {
        perror("mytouch");
        return 1;
    }
    
    close(fd);
    return 1;
}

int run_external(char **args) {
    pid_t pid;
    int status;
    int exec_result;
    int i = 0;
    int fd;

    pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);

        while (args[i] != NULL) {
            if (strcmp(args[i], ">") == 0) {
                if (args[i+1] == NULL) {
                    fprintf(stderr, "%sBrak nazwy pliku po >%s\n", COL_BORDO, COL_RESET);
                    exit(EXIT_FAILURE);
                }
                fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open redirection failed");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
            i++;
        }

        exec_result = execvp(args[0], args);
        if (exec_result == -1) {
            fprintf(stderr, "%sNieznane polecenie: %s%s\n", COL_BORDO, args[0], COL_RESET);
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed");
    } else {
        wait(&status);
    }
    return 1;
}

int main() {
    char cmd_buf[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int status = 1;
    char *input_result;

    signal(SIGINT, handle_sigint);
    print_intro();

    while (status) {
        print_prompt();

        input_result = fgets(cmd_buf, sizeof(cmd_buf), stdin);
        
        if (input_result == NULL) {
            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            } else {
                printf("\n");
                break;
            }
        }
        cmd_buf[strcspn(cmd_buf, "\n")] = 0;

        if (strlen(cmd_buf) == 0) {
            continue;
        }

        int i = 0;
        args[i] = strtok(cmd_buf, " \t\n");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " \t\n");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            status = run_exit(args);
        } else if (strcmp(args[0], "help") == 0) {
            status = run_help(args);
        } else if (strcmp(args[0], "cd") == 0) {
            status = run_cd(args);
        } else if (strcmp(args[0], "mycp") == 0) {
            status = run_mycp(args);
        } else if (strcmp(args[0], "mytouch") == 0) {
            status = run_mytouch(args);
        } else {
            status = run_external(args);
        }
    }

    return 0;
}