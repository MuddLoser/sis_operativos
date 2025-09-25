#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define T_MAX 1024
#define T_MAX_ARG 100

void ejecutar_comando(char **args) {
    if (execvp(args[0], args) == -1) {
        perror("Error ejecutando el comando");
    }
    exit(EXIT_FAILURE);
}

void manejar_pipes(char *input) {
    int fd[2];
    pid_t pid;
    int estado;
    char *comandos[10];
    char *args[T_MAX_ARG];
    int i = 0;

    comandos[i] = strtok(input, "|");
    while (comandos[i] != NULL) {
        i++;
        comandos[i] = strtok(NULL, "|");
    }

    int num_comandos = i;
    int in_fd = 0;

    for (int j = 0; j < num_comandos; j++) {
        pipe(fd);
        pid = fork();
        if (pid == -1) {
            perror("Error al crear el proceso hijo");
        } else if (pid == 0) {
            dup2(in_fd, STDIN_FILENO);
            if (j < num_comandos - 1) {
                dup2(fd[1], STDOUT_FILENO);
            }
            close(fd[0]);

            int k = 0;
            args[k] = strtok(comandos[j], " ");
            while (args[k] != NULL) {
                k++;
                args[k] = strtok(NULL, " ");
            }

            ejecutar_comando(args);
        } else {
            wait(&estado);
            close(fd[1]);
            in_fd = fd[0];
        }
    }
}

void shell() {
    char input[T_MAX];
    char *args[T_MAX_ARG];
    char *temp;
    int i;
    pid_t pid;
    int estado;

    while (1) {
        printf("shell:$ ");
        fflush(stdout);

        if (fgets(input, T_MAX, stdin) == NULL) {
            perror("Error leyendo la entrada");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        if (strcmp(input, "exit") == 0) {
            printf("Saliendo de la shell...\n");
            break;
        }

        if (strchr(input, '|') != NULL) {
            manejar_pipes(input);
            continue;
        }

        i = 0;
        temp = strtok(input, " ");
        while (temp != NULL) {
            args[i] = temp;
            i++;
            temp = strtok(NULL, " ");
        }
        args[i] = NULL;

        pid = fork();
        if (pid == -1) {
            perror("Error al crear el proceso hijo");
        } else if (pid == 0) {
            ejecutar_comando(args);
        } else {
            wait(&estado);
        }
    }
}

int main() {
    shell();
    return 0;
}
