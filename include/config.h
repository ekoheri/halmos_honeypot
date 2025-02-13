#ifndef CONFIG_H
#define CONFIG_H

// Struktur data untuk konfigurasi
typedef struct {
    char server_name[256];
    int server_port;
    char web_server_name[256];
    int web_server_port;
    int max_event;
    char python_version[256];
    char python_directory[256];
    char python_script[256];
} Config;

// Deklarasikan variabel global untuk menyimpan konfigurasi
extern Config config;

//Fungsi untuk membersihkan spasi diawal dan diakhir string
char *trim(char *str);

// Fungsi untuk memuat konfigurasi dari file
void load_config(const char *filename);

#endif