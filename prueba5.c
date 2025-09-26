#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

#define T_MAX 1024
#define T_MAX_ARG 100

pid_t pid_global = -1;

void manejador_alarma(int sig) {
    if (pid_global > 0 && kill(pid_global, 0) == 0) {
        kill(pid_global, SIGKILL);
        printf("Tiempo límite alcanzado. Proceso terminado.\n");
    }
}

void ejecutar_comando(char **args) {
    if (execvp(args[0], args) == -1) {
        perror("Error ejecutando el comando");
    }
    _exit(EXIT_FAILURE);
}

void miprof(char *modo, char *archivo, int tiempo_max, char **comando) {
    struct rusage uso;
    struct timeval inicio, fin;
    int estado;

    signal(SIGALRM, manejador_alarma);
    gettimeofday(&inicio, NULL);
    pid_t pid = fork();
    pid_global = pid;

    if (pid == -1) {
        perror("Error al crear el proceso hijo");
        return;
    } else if (pid == 0) {
        ejecutar_comando(comando);
    } else {
        if (tiempo_max > 0) {
            alarm(tiempo_max);
        }

        wait4(pid, &estado, 0, &uso);
        gettimeofday(&fin, NULL);
        alarm(0);

        double tiempo_real = (fin.tv_sec - inicio.tv_sec) + (fin.tv_usec - inicio.tv_usec) / 1e6;
        double tiempo_usuario = uso.ru_utime.tv_sec + uso.ru_utime.tv_usec / 1e6;
        double tiempo_sistema = uso.ru_stime.tv_sec + uso.ru_stime.tv_usec / 1e6;
        long memoria_max = uso.ru_maxrss;

        char resultado[1024];
        snprintf(resultado, sizeof(resultado),
            "Comando: %s\nTiempo real: %.6f s\nTiempo usuario: %.6f s\nTiempo sistema: %.6f s\nMemoria máxima: %ld KB\n\n",
            comando[0], tiempo_real, tiempo_usuario, tiempo_sistema, memoria_max);

        if (strcmp(modo, "ejec") == 0 || strcmp(modo, "maxtiempo") == 0) {
            printf("%s", resultado);
        } else if (strcmp(modo, "ejecsave") == 0 && archivo != NULL) {
            FILE *f = fopen(archivo, "a");
            if (f == NULL) {
                perror("Error abriendo el archivo");
                return;
            }
            fprintf(f, "%s", resultado);
            fclose(f);
        }
    }
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
            args[k] = NULL;

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

    while (1) {
        printf("shell:$ ");
        fflush(stdout);

        if (fgets(input, T_MAX, stdin) == NULL) {
            perror("Error leyendo la entrada");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;
        if (strcmp(input, "exit") == 0) break;

        if (strncmp(input, "miprof", 6) == 0) {
            char *modo = strtok(input + 7, " ");
            if (modo == NULL) {
                printf("Uso: miprof [ejec|ejecsave archivo|maxtiempo segundos] comando args\n");
                continue;
            }

            char *archivo = NULL;
            int tiempo_max = 0;
            char *comando[T_MAX_ARG];
            int j = 0;

            if (strcmp(modo, "ejecsave") == 0) {
                archivo = strtok(NULL, " ");
                if (archivo == NULL) {
                    printf("Error: Debes proporcionar el nombre del archivo.\n");
                    continue;
                }
            } else if (strcmp(modo, "maxtiempo") == 0) {
                char *tiempo_str = strtok(NULL, " ");
                if (tiempo_str == NULL) {
                    printf("Error: Debes proporcionar el tiempo máximo en segundos.\n");
                    continue;
                }
                tiempo_max = atoi(tiempo_str);
            }

            char *token = strtok(NULL, " ");
            if (token == NULL) {
                printf("Error: Debes proporcionar un comando para ejecutar.\n");
                continue;
            }

            while (token != NULL) {
                comando[j++] = token;
                token = strtok(NULL, " ");
            }
            comando[j] = NULL;

            miprof(modo, archivo, tiempo_max, comando);
            continue;
        }

        if (strchr(input, '|') != NULL) {
            manejar_pipes(input);
            continue;
        }

        // Soporte para redirección >
        char *archivo_salida = NULL;
        int redir = 0;
        i = 0;
        temp = strtok(input, " ");
        while (temp != NULL) {
            if (strcmp(temp, ">") == 0) {
                redir = 1;
                temp = strtok(NULL, " ");
                archivo_salida = temp;
                break;
            }
            args[i++] = temp;
            temp = strtok(NULL, " ");
        }
        args[i] = NULL;

        pid_t pid = fork();
        if (pid == -1) {
            perror("Error al crear el proceso hijo");
        } else if (pid == 0) {
            if (redir && archivo_salida != NULL) {
                int fd = open(archivo_salida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("Error abriendo archivo de salida");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            ejecutar_comando(args);
        } else {
            int estado;
            wait(&estado);
        }
    }
}

int main() {
    shell();
    return 0;
}
