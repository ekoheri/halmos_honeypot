#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "../include/config.h"

// Inisialisasi variabel global config
Config config = {0};

char *trim(char *str) {
    char *end;

    // Hapus spasi di depan
    while (isspace((unsigned char)*str)) {
        str++;
    }

    // Jika hanya ada spasi
    if (*str == 0) {
        return str;
    }

    // Hapus spasi di belakang
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Tambahkan null-terminator setelah karakter terakhir yang bukan spasi
    *(end + 1) = '\0';

    return str;
}

// Fungsi untuk membaca file konfigurasi
void load_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening config file");
        return;
    }

    char line[1024];  // Buffer untuk membaca setiap baris
    char section[256];  // Menyimpan nama section (misalnya [Performance])

    while (fgets(line, sizeof(line), file)) {
        // Menghapus karakter newline di akhir baris
        line[strcspn(line, "\n")] = '\0';

        // Lewati komentar
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }

        // Menangani bagian [Performance] (atau bagian lainnya)
        if (line[0] == '[') {
            sscanf(line, "[%255[^]]]", section);  // Menyimpan nama section
            continue;
        }

        // Mencari tanda "=" yang memisahkan key dan value
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if (key != NULL && value != NULL) {
            // Hapus spasi di sekitar key dan value
            key = trim(key);
            value = trim(value);

            // Menangani key tertentu
            if (strcmp(key, "server_name") == 0) {
                strncpy(config.server_name, value, sizeof(config.server_name));
            } else if (strcmp(key, "server_port") == 0) {
                config.server_port = atoi(value);  // Konversi ke integer
            } else if (strcmp(key, "web_server_name") == 0) {
                strncpy(config.web_server_name, value, sizeof(config.web_server_name));
            } else if (strcmp(key, "web_server_port") == 0) {
                config.web_server_port = atoi(value);
            } else if (strcmp(key, "max_event") == 0) {
                config.max_event = atoi(value);
            } else if (strcmp(key, "python_version") == 0) {
                strncpy(config.python_version, value, sizeof(config.python_version));
            } else if (strcmp(key, "python_directory") == 0) {
                strncpy(config.python_directory, value, sizeof(config.python_directory));
            } else if (strcmp(key, "python_script") == 0) {
                strncpy(config.python_script, value, sizeof(config.python_script));
            }
        }
    }

    fclose(file);
}
