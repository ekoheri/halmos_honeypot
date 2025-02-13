#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h> // string manipulation
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <curl/curl.h>

#include "../include/handle_request.h"
#include "../include/config.h"
#include "../include/log.h"

#define BUFFER_SIZE 1024

extern Config config;

// Fungsi untuk mengonversi hex menjadi karakter
char hex_to_char(char first, char second) {
    char hex[3] = {first, second, '\0'};
    return (char) strtol(hex, NULL, 16);
}

// Fungsi URL decoding
void url_decode(const char *src, char *dest) {
    while (*src) {
        if (*src == '%') {
            if (isxdigit(src[1]) && isxdigit(src[2])) {
                *dest++ = hex_to_char(src[1], src[2]);
                src += 3;  // Lewati %xx
            } else {
                *dest++ = *src++;
            }
        } else if (*src == '+') {
            *dest++ = ' ';  // Konversi '+' menjadi spasi
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';  // Null-terminate string hasil decode
}

RequestHeader parse_request_line(char *request) {
    RequestHeader req_header = {0}; // Inisialisasi semua field ke 0
    const char *line = request;
    int line_count = 0;

    while (line && *line) {
        const char *next_line = strstr(line, "\r\n");
        size_t line_length = next_line ? (size_t)(next_line - line) : strlen(line);
        
        if (line_length == 0) {
            line = next_line ? next_line + 2 : NULL;
            break;
        }

        char *line_copy = strndup(line, line_length);

        if (line_count == 0) { // Request Line (Baris Pertama)
            char *words[3] = {NULL, NULL, NULL};
            int i = 0;
            char *token = strtok(line_copy, " ");

            while (token && i < 3) {
                words[i++] = token;
                token = strtok(NULL, " ");
            }

            if (i == 3) {
                strncpy(req_header.method, words[0], sizeof(req_header.method) - 1);
                req_header.uri = strdup(words[1]);
                strncpy(req_header.http_version, words[2], sizeof(req_header.http_version) - 1);

                // Hapus tanda '/' pada URI jika ada
                if (req_header.uri[0] == '/') {
                    memmove(req_header.uri, req_header.uri + 1, strlen(req_header.uri));
                }

                // Pisahkan query string jika ada
                char *query_start = strchr(req_header.uri, '?');
                if (query_start) {
                    *query_start = '\0';  // Pisahkan URI dan Query String
                    char *query_decoded = malloc(strlen(query_start + 1) + 1);
                    if (query_decoded) {
                        url_decode(query_start + 1, query_decoded);
                        req_header.query_string = query_decoded;
                    } else {
                        req_header.query_string = strdup(""); // Jika alokasi gagal
                    }
                } else {
                    req_header.query_string = strdup(""); // Jika tidak ada query string
                }
            }
        } else { // Header Lines
            if (strncmp(line_copy, "Encrypted: ", 11) == 0) {
                req_header.encrypted = strdup(line_copy + 11);
            } else if (strncmp(line_copy, "Request-Time: ", 14) == 0) {
                req_header.request_time = strdup(line_copy + 14);
            } else if (strncmp(line_copy, "Content-Length: ", 16) == 0) {
                req_header.content_length = atoi(line_copy + 16);
            }
        }

        free(line_copy);
        line = next_line ? next_line + 2 : NULL; // Pindah ke baris berikutnya
        line_count++;
    }

    // Jika masih ada data setelah header (body)
    if (line && *line) {
        char *body_decoded = malloc(strlen(line) + 1);
        if (body_decoded) {
            url_decode(line, body_decoded);
            req_header.body_data = body_decoded;
        } else {
            req_header.body_data = strdup(""); // Jika alokasi gagal
        }
    }  else {
        req_header.body_data = strdup(""); // Jika tidak ada body data
    }

    // Jika URI kosong, gunakan "index.html"
    if (!req_header.uri || strlen(req_header.uri) == 0) {
        free(req_header.uri);
        req_header.uri = strdup("index.html");
    }

    return req_header;
}

char *get_time_string() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // Mengambil waktu dalam format struct tm (GMT)
    struct tm *tm_info = gmtime(&tv.tv_sec);

    // Alokasikan buffer yang cukup besar untuk waktu dan milidetik
    char *buf = (char *)malloc(64);  // Ukuran buffer yang memadai
    if (!buf) return NULL;  // Cek jika malloc gagal

    // Format waktu tanpa milidetik terlebih dahulu
    strftime(buf, 64, "%a, %d %b %Y %H:%M:%S", tm_info);
    
    // Tambahkan milidetik ke string
    int millis = tv.tv_usec / 1000;
    snprintf(buf + strlen(buf), 64 - strlen(buf), ".%03d GMT", millis);

    return buf;
}

char *generate_response_header(ResponseHeader res_header) {
    char *header = malloc(BUFFER_SIZE);
    if (header == NULL) {
        return NULL;
    }

    snprintf(
        header, BUFFER_SIZE,
        "%s %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lu\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-cache\r\n"
        "Response-Time: %s\r\n"
        "Encrypted: %s\r\n"
        "\r\n",  // Akhir dari header
        res_header.http_version, res_header.status_code, 
        res_header.status_message, res_header.mime_type, 
        res_header.content_length, res_header.response_time, 
        res_header.encrypted);
    
    return header;
} // generate_response_header

char *create_response(
    int *response_size, 
    ResponseHeader *res_header, 
    const char *body, int body_size) {
    
    char *response = NULL;

    // Generate response header
    res_header->content_length = body_size;
    char *response_header = generate_response_header(*res_header);
    if (!response_header) return NULL;

    // Allocate memory for full response
    int header_size = strlen(response_header);
    response = (char *)malloc(header_size + body_size);
    if (response) {
        memcpy(response, response_header, header_size);   // Copy header
        memcpy(response + header_size, body, body_size);  // Copy body
        *response_size = header_size + body_size;
    }

    free(response_header);

    //log
    write_log(" * Response : %s %d %s", res_header->http_version, res_header->status_code, res_header->status_message);
    return response;
}

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *data) {
    MemoryStruct *mem = (MemoryStruct *)data;
    size_t total_size = size * nmemb;

    char *new_memory = realloc(mem->memory, mem->size + total_size + 1);
    if (!new_memory) {
        perror("Error allocating memory");
        return 0;
    }

    mem->memory = new_memory;
    memcpy(&(mem->memory[mem->size]), ptr, total_size);
    mem->size += total_size;
    mem->memory[mem->size] = '\0';  // Null-terminate string

    return total_size;
}

MemoryStruct forward_request(RequestHeader req_header) {
    MemoryStruct response = {NULL, 0, NULL};  // Inisialisasi MemoryStruct

    // Inisialisasi CURL
    response.curl = curl_easy_init();
    if (!response.curl) {
        //fprintf(stderr, "Error initializing CURL\n");
        write_log("Gagal Menginisialisasi CURL CURL");
        return response;
    }

    // URL lengkap dari request URI
    char full_url[1024];
    //snprintf(full_url, sizeof(full_url), "%s:%d/%s", config.web_server_name, config.web_server_port, req_header.uri);
    snprintf(full_url, sizeof(full_url), "%s/%s", config.web_server_name, req_header.uri);

    // Set opsi CURL
    curl_easy_setopt(response.curl, CURLOPT_URL, full_url);
    curl_easy_setopt(response.curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(response.curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(response.curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Jika metode adalah POST atau PUT, kirim body request
    if (strcmp(req_header.method, "POST") == 0 || strcmp(req_header.method, "PUT") == 0) {
        curl_easy_setopt(response.curl, CURLOPT_POSTFIELDS, req_header.body_data);
        curl_easy_setopt(response.curl, CURLOPT_POSTFIELDSIZE, req_header.content_length);
    }

    // Set metode HTTP sesuai dengan request
    if (strcmp(req_header.method, "POST") == 0) {
        curl_easy_setopt(response.curl, CURLOPT_POST, 1L);
    } else if (strcmp(req_header.method, "PUT") == 0) {
        curl_easy_setopt(response.curl, CURLOPT_CUSTOMREQUEST, "PUT");
    } else if (strcmp(req_header.method, "DELETE") == 0) {
        curl_easy_setopt(response.curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    // Eksekusi request ke server target
    CURLcode res = curl_easy_perform(response.curl);
    if (res != CURLE_OK) {
        //fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
         write_log("CURL error: %s", curl_easy_strerror(res));
    }

    return response;  // Kembalikan MemoryStruct yang berisi body dan metadata respons
}

int run_python_script(
    const char *query_string, 
    const char *body_data) {

    int attack_detected = 0;  // Flag untuk menandakan ada error
    char command[BUFFER_SIZE];
    FILE *fp;
    int response_buffer_size = 8 * BUFFER_SIZE;

    // Menjalankan skrip PHP dengan GET dan POST
    snprintf(command, sizeof(command),
             "%s %s%s \"%s%s\"",
             config.python_version, 
             config.python_directory, 
             config.python_script, 
             query_string, body_data
    );
    //printf("Perintah : %s\n", command);
    // Buffer untuk menyimpan seluruh respons
    char *response = (char *)malloc(response_buffer_size);
    if (response == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    response[0] = '\0';  // Inisialisasi string kosong

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        free(response);
        exit(EXIT_FAILURE);
    }

    char result[response_buffer_size];

    // Membaca output dari PHP dan menyusunnya ke dalam response buffer
    while (fgets(result, sizeof(result), fp) != NULL) {
        // Cek apakah ada pesan kesalahan dari PHP
        if (strstr(result, "OUTPUT: True")) {
            attack_detected = 1;  // Tandai bahwa ada error
        } 
    }

    if (pclose(fp) == -1) {
        perror("pclose");
        free(response);
        exit(EXIT_FAILURE);
    }

    return attack_detected;
}

char *handle_method(int *response_size, RequestHeader req_header) {
    char *response = NULL;

    // Jika request tidak valid, kembalikan 400 Bad Request
    if (strlen(req_header.method) == 0 || strlen(req_header.uri) == 0 || strlen(req_header.http_version) == 0) {
        ResponseHeader res_header = {
            .http_version = "HTTP/1.1",
            .status_code = 400,
            .status_message = "Bad Request",
            .mime_type = "text/html",
            .content_length = 0
        };

        char *_400 = "<h1>400 Bad Request</h1>";
        response = create_response(response_size, &res_header, _400, strlen(_400));
        return response;
    }

    //Sebelum check_attack deteksi ekstensi file dulu
    int check_attack = 0;
    const char *extension = strrchr(req_header.uri, '.');
    if ( (extension && strcmp(extension, ".php") == 0)  || 
         (extension && strcmp(extension, ".js") == 0)
    ){
        check_attack = run_python_script(
        req_header.query_string, 
        req_header.body_data);
    }
    
    if(check_attack == 0) {
        // 1. Panggil `forward_request`
        MemoryStruct forward_res = forward_request(req_header);
        long http_code = 0;
        char *content_type = NULL;
        long http_version_curl = 0;

        // 2. Ambil informasi dari CURL
        curl_easy_getinfo(forward_res.curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_getinfo(forward_res.curl, CURLINFO_CONTENT_TYPE, &content_type);
        curl_easy_getinfo(forward_res.curl, CURLINFO_HTTP_VERSION, &http_version_curl);

        // 3. Konversi versi HTTP dari CURL ke string
        const char *http_version_str = "HTTP/1.1"; // Default
        if (http_version_curl == CURL_HTTP_VERSION_2_0) {
            http_version_str = "HTTP/2";
        } else if (http_version_curl == CURL_HTTP_VERSION_1_0) {
            http_version_str = "HTTP/1.0";
        }

        // 4. Ambil status message berdasarkan status_code
        const char *status_message = "OK"; // Default
        if (http_code == 400) status_message = "Bad Request";
        else if (http_code == 403) status_message = "Forbidden";
        else if (http_code == 404) status_message = "Not Found";
        else if (http_code == 500) status_message = "Internal Server Error";

        // 5. Buat ResponseHeader berdasarkan hasil `forward_request`
        ResponseHeader res_header = {
            .http_version = http_version_str,
            .status_code = (int)http_code,
            .status_message = status_message,
            .mime_type = content_type ? content_type : "application/octet-stream",
            .content_length = forward_res.size,
            .response_time = get_time_string(),
            .encrypted = "false"
        };

        // 6. Buat response dengan `create_response`
        response = create_response(response_size, &res_header, forward_res.memory, forward_res.size);

        // 7. Bebaskan memory dari `forward_request`
        curl_easy_cleanup(forward_res.curl);
        free(forward_res.memory);

        return response;
    } else {
         ResponseHeader res_header = {
            .http_version = "HTTP/1.1",
            .status_code = 200,
            .status_message = "OK",
            .mime_type = "text/html",
            .content_length = 0
        };

        char *_attack = "<div style = \"padding: 10px; margin: 10px; border: 1px solid #ccc;text-align: center; background-color: maroon;color: white;\">"
        "<h1>Kamu terdeteksi ingin membobol server!</h1>"
        "<h2>Sistem ini telah dilengkapi dengan AI,</h2>"
        "<h2>untuk menangkal serangan jahat dari kamu!</h2>"
        "</div>";
        response = create_response(response_size, &res_header, _attack, strlen(_attack));
        return response;
    }
}

