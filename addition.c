#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 100

int main(int argc, char *argv[]) {
    if (argc != 3) {
    printf("Usage: %s <input_fifo> <output_fifo>\n", argv[0]);
    return EXIT_FAILURE;
}

    char *input_fifo = argv[1];
    char *output_fifo = argv[2];
    char buffer[BUFFER_SIZE];

    // Giriş FIFO'sını aç
    int fd_in = open(input_fifo, O_RDONLY);
    if (fd_in == -1) {
        perror("Failed to open input FIFO");
        return EXIT_FAILURE;
    }

    // Çıkış FIFO'sını aç
    int fd_out = open(output_fifo, O_WRONLY);
    if (fd_out == -1) {
        perror("Failed to open output FIFO");
        close(fd_in);
        return EXIT_FAILURE;
    }

    while (1) {
        ssize_t bytes_read = read(fd_in, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                usleep(100000); // FIFO boşsa bekle
                continue;
            } else {
                perror("Read error");
                break;
            }
        }

        buffer[bytes_read] = '\0'; // Sonlandırıcı ekle

        int a, b;
        if (sscanf(buffer, "%d %d", &a, &b) != 2) {
            snprintf(buffer, sizeof(buffer), "Error: Invalid input, expected two integers");
        } else {
            // Toplama işlemi
            int result = a + b;
            snprintf(buffer, sizeof(buffer), "%d", result);
        }

        // Çıkış FIFO'sına yaz
        if (write(fd_out, buffer, strlen(buffer) + 1) == -1) {
            perror("Failed to write to output FIFO");
            break;
        }
    }

    close(fd_in);
    close(fd_out);
    return 0;
}
