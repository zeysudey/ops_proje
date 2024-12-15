#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 100
#define FIFO_COUNT 4
#define FIFO_NAME_SIZE 20

char input_fifo[FIFO_COUNT][FIFO_NAME_SIZE];
char output_fifo[FIFO_COUNT][FIFO_NAME_SIZE];

void cleanup_fifos() {
    for (int i = 0; i < FIFO_COUNT; i++) {
        unlink(input_fifo[i]);
        unlink(output_fifo[i]);
    }
}

int main() {
    char buffer[BUFFER_SIZE];
    pid_t pids[FIFO_COUNT];

    // FIFO create
    for (int i = 0; i < FIFO_COUNT; i++) {
        snprintf(input_fifo[i], FIFO_NAME_SIZE, "input_fifo_%d", i);
        snprintf(output_fifo[i], FIFO_NAME_SIZE, "output_fifo_%d", i);

        unlink(input_fifo[i]);
        unlink(output_fifo[i]);

        if (mkfifo(input_fifo[i], 0666) == -1) {
            perror("Failed to create input FIFO");
            cleanup_fifos();
            return 1;
        }

        if (mkfifo(output_fifo[i], 0666) == -1) {
            perror("Failed to create output FIFO");
            cleanup_fifos();
            return EXIT_FAILURE;
        }
    }

    // Çocuk süreçleri oluştur
    for (int i = 0; i < FIFO_COUNT; i++) {
        pids[i] = fork();

        if (pids[i] == -1) {
            perror("Fork failed");
            cleanup_fifos();
            return 1;
        }

        if (pids[i] == 0) { // Çocuk süreç
            execlp(i == 0 ? "./addition" : i == 1 ? "./subtraction" : i == 2 ? "./multiplication" : "./division", 
                   i == 0 ? "addition" : i == 1 ? "subtraction" : i == 2 ? "multiplication" : "division",
                   input_fifo[i], output_fifo[i], NULL);

            perror("execlp failed");
            exit(EXIT_FAILURE);
        }
    }

    // Kullanıcı işlemleri
    while (1) {
        printf("Calculator\n");
        printf("1- Addition\n2- Subtraction\n3- Multiplication\n4- Division\n5- Exit\n");
        printf("Select an operation: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input, please enter a valid choice.\n");
            continue;
        }

        if (choice == 5) {
            printf("Exiting...\n");
            // Çocuk süreçleri sonlandır
            for (int i = 0; i < FIFO_COUNT; i++) {
                kill(pids[i], SIGTERM);
                waitpid(pids[i], NULL, 0);
            }
            cleanup_fifos();
            break;
        }

        if (choice < 1 || choice > 4) {
            printf("Invalid selection. Try again.\n");
            continue;
        }

        int a, b;
        printf("Enter two numbers: ");
        if (scanf("%d %d", &a, &b) != 2) {
            while (getchar() != '\n');
            printf("Invalid numbers, please try again.\n");
            continue;
        }

        snprintf(buffer, BUFFER_SIZE, "%d %d", a, b);

        // Giriş FIFO'sına yazma
        int fd_in = open(input_fifo[choice - 1], O_WRONLY);
        if (fd_in == -1) {
            perror("Failed to open input FIFO for writing");
            continue;
        }

        if (write(fd_in, buffer, strlen(buffer) + 1) == -1) {
            perror("Failed to write to input FIFO");
            close(fd_in);
            continue;
        }

        close(fd_in);  // Giriş FIFO'sını kapat

        // Çıkış FIFO'sından okuma
        memset(buffer ,0 ,BUFFER_SIZE);

        int fd_out = open(output_fifo[choice - 1], O_RDONLY);
        if (fd_out == -1) {
            perror("Failed to open output FIFO for reading");
            continue;
        }

        ssize_t bytesRead = read(fd_out, buffer, BUFFER_SIZE);
        if (bytesRead <= 0) {
            perror("Failed to read from output FIFO");
            close(fd_out);
            continue;
        }

        buffer[bytesRead] = '\0';
        printf("Result: %s\n", buffer);

        close(fd_out);                


        // Saver işlemi
        pid_t saver_pid = fork();
        if (saver_pid == 0) {
            // Çocuk süreçte saver.c'yi çalıştırıyoruz ve parametre olarak sonucu veriyoruz.
            execlp("./saver", "saver", buffer, NULL);
            perror("execlp failed");
            exit(1);
        } else if (saver_pid < 0) {
            perror("Failed to fork saver process");
        } else {
            // Saver sürecinin tamamlanmasını bekle
            waitpid(saver_pid, NULL, 0);
        }
    }
cleanup_fifos();
    return 0;
}
