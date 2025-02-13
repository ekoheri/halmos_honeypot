#ifndef HANDLE_REQUEST_H
#define HANDLE_REQUEST_H

#include <curl/curl.h>

typedef struct {
    char method[16];
    char *uri;
    char http_version[16];
    char *query_string;
    char *body_data;
    char *request_time;
    char *encrypted;
    int content_length;
} RequestHeader;

typedef struct {
    const char *http_version;
    int status_code;
    const char *status_message;
    const char *mime_type;
    unsigned long content_length;
    const char *response_time;
    const char *encrypted;
} ResponseHeader;

typedef struct {
    char *memory;   // Buffer untuk menyimpan data respons
    size_t size;    // Ukuran data respons
    CURL *curl;     // Simpan pointer ke CURL untuk mengambil metadata respons
} MemoryStruct;

RequestHeader parse_request_line(char *request);

char *handle_method(int *response_size, RequestHeader req_header);
#endif