#include "flasc.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>

#define HTTP_VERSION "HTTP/1.0"
#define CRLF "\r\n"
#define SP " "

#define response_status_line_cap 1024 // bytes
#define response_header_cap 1024 // bytes
#define response_body_cap 2048 // bytes

enum STATUS_CODE {
    OK = 200,
    BAD_REQUEST = 400,
    FILE_NOT_FOUND = 404,
    SERVER_ERROR = 500
};

struct http_response{
    enum STATUS_CODE status;

    string rep_file;
    string file_type;
    
    string status_line;
    string header;
    string body;
};


int request_parser(string *request, struct http_response *response);
char *get_mime(char *file_extension);

int response_statusline_builder(struct http_response *response);
int response_header_builder(struct http_response *response);
int response_body_builder(struct http_response *response);

int http_handler(string *request, string *output)
{   
    struct http_response response;
    enum STATUS_CODE status;

    // initializing response struct DONT FORGET TO FREE AFTER !
    if (string_builder(response_status_line_cap, &response.rep_file) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }
    if (string_builder(response_status_line_cap, &response.file_type) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }
    if (string_builder(response_status_line_cap, &response.status_line) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }
    if (string_builder(response_header_cap, &response.header) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }
    if (string_builder(response_body_cap, &response.body) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }

    // validating request line
    request_parser(request, &response);

    // assembling response string
    response_statusline_builder(&response);
    response_header_builder(&response);
    response_body_builder(&response);

    // sending assembled response to accept worker
    if (string_append(response.status_line.data, output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string error\n");
        return -1;
    }
    if (string_append(response.header.data, output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string error\n");
        return -1;
    }
    if (response.status == OK) {
        if (string_append(response.body.data, output) == STRING_ERROR) {
            fprintf(stderr, "assembling response string error\n");
            return -1;
        }
    }
    // cleanup
    delete_string(&response.rep_file);
    delete_string(&response.file_type);
    delete_string(&response.status_line);
    delete_string(&response.header);
    delete_string(&response.body);
    return 0;
};

// valdiating request
int request_parser(string *request, struct http_response *response)
{
    char *save_line, *save_element, *element, *end;;
    // isolating request line
    char *request_line = strtok_r(request->data, CRLF, &save_line);

    if (!request_line || !request->data) {
        response->status = BAD_REQUEST;
        return -1;
    }
    // method validation
    element = strtok_r(request_line, SP, &save_element);
    if (strcmp(element, "GET") != 0) { // expand for POST request
        response->status = BAD_REQUEST;
        return -1;
    } 
    // URI validation
    element = strtok_r(NULL, SP, &save_element);
    if (element[0] != '/') {
        return response->status = BAD_REQUEST;
    }
    int uri_len = strlen(element);
    // mapping index.html to "/"
    if (strcmp(element, "/index.html") == 0) {
        char *filename = node_search("/", &ht);
        string_append(filename, &response->rep_file); 
        string_append("html", &response->file_type);
        // version validation
        element = strtok_r(NULL, SP, &save_element);
        if (strcmp(element, HTTP_VERSION) != 0 && strcmp(element, "HTTP/1.1") != 0 ) {
            return response->status = BAD_REQUEST;
        } 
        return response->status = OK;
    }
    // extracing file type
    end = strrchr(element, '.');
    if (end != NULL) {
        char *mime = get_mime(end);
        if (!mime) {
            response->status = FILE_NOT_FOUND;
            return -1;
        }
        string_append(mime, &response->file_type);
    }
    // truncating to "/route"
    if (end != NULL) {
        *end = '\0';
    } 

    char *filename = node_search(element, &ht);
    if (!filename) {
        response->status = FILE_NOT_FOUND;
        return -1;
    }
    string_append(filename, &response->rep_file); 

    // version validation
    element = strtok_r(NULL, SP, &save_element);
    if (strcmp(element, HTTP_VERSION) != 0 && strcmp(element, "HTTP/1.1") != 0 ) {
        return response->status = BAD_REQUEST;
    } 
    response->status = OK;
    return 0;
}

char *get_mime(char *file_extension)
{
    if (strcmp(file_extension, ".html") == 0) return "text/html";
    if (strcmp(file_extension, ".css") == 0) return "text/css";
    if (strcmp(file_extension, ".js") == 0) return "text/javascript";
    return NULL;
}
//status line builder takes status code and response lne string to edit
int response_statusline_builder(struct http_response *response)
{
    
    if (string_append(HTTP_VERSION, &response->status_line) == STRING_ERROR) {
        fprintf(stderr, "response head builder string error\n");
        return -1;
    }
    switch (response->status) {
        case OK:
            if (string_append(" 200 OK", &response->status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
        case BAD_REQUEST:
            if (string_append(" 400 Bad Request", &response->status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
        case FILE_NOT_FOUND:
            if (string_append(" 404 Not Found", &response->status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
        case SERVER_ERROR:
            if (string_append(" 500 Internal Server Error", &response->status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
    }
    if (string_append(CRLF, &response->status_line) == STRING_ERROR) {
        fprintf(stderr, "response head builder string error\n");
        return -1;
    }
    return 0;
}

// header crafter
int response_header_builder(struct http_response *response)
{
    int header_field_amnt = 3;
    char *header_fields[header_field_amnt];
    header_fields[0] = "Connection: close\r\n";
    header_fields[1] = "Server: FlascServer\r\n";

    char content_type[100];

    strcat(content_type, "Content-Type: ");
    strcat(content_type, response->file_type.data);
    strcat(content_type, "\r\n");

    header_fields[2] = content_type;

    for (int i = 0; i < header_field_amnt; i++) {
        if (string_append(header_fields[i], &response->header) == STRING_ERROR) {
            fprintf(stderr, "assembling response header string error\n");
            return -1;
        }
    }
    return 0;
}   

// body crafter
int response_body_builder(struct http_response *response)
{
    // assembling file path
    char pagePath[MAX_PATH];
    snprintf(pagePath, sizeof(pagePath), "%s\\static\\%s", rootpath, response->rep_file.data);

    FILE *page_file = fopen(pagePath, "r");
    if (!page_file) return -1;

    fseek(page_file, 0, SEEK_END);
    long size = ftell(page_file); // determining file size
    fseek(page_file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    if (!buffer) return -1;

    size_t read_bytes = fread(buffer, 1, size, page_file);
    buffer[read_bytes] = '\0';

    if (string_append(CRLF, &response->body) == STRING_ERROR) { // carriage return new line to indicate body
        fprintf(stderr, "response body builder string error\n");
        return -1;
    }
    if (string_append(buffer, &response->body) == STRING_ERROR) {
        fprintf(stderr, "response body builder string error\n");
        return -1;
    }
    fclose(page_file);
    free(buffer);
    return 0;
}
