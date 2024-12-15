#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <result>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parametre olarak alınan sonucu dosyaya yazalım
    const char *result = argv[1];
    FILE *file = fopen("results.txt", "a");
    if (file == NULL) {
        perror("Failed to open result file");
        return EXIT_FAILURE;
    }

    fprintf(file, "%s\n", result);
    fclose(file);
    printf("Saver process saved result: %s\n", result);

    return EXIT_SUCCESS;
}
