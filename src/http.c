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

enum MIME {
    html,
    css
};

struct http_response{
    enum STATUS_CODE;
    enum MIME;
    string status_line;
    string header;
    string body;
};

enum REQUEST_FLAG {
    html,
    css
};

enum STATUS_CODE request_header_validator(char *request_line);
int response_statusline_builder(enum STATUS_CODE STATUS_CODE, string *response);
int response_header_builder(string *header);
int response_body_builder(string *body, char *key);

int http_handler(string *request, string *output)
{   
    struct http_response response;
    enum STATUS_CODE status;
    char *key;

    // initializing response struct DONT FORGET TO FREE AFTER !
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
    char *save_line;
    char *token_line = strtok_r(request->data, CRLF, &save_line); //checks for formatting of first lien
    status = request_header_validator(token_line);   

    // assembling response string
    response_statusline_builder(status, &response.status_line);
    response_header_builder(&response.header);
    response_body_builder(&response.body);

    // sending assembled response to accept worker
    if (string_append(response.status_line.data, output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string error\n");
        return -1;
    }
    if (string_append(response.header.data, output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string error\n");
        return -1;
    }
    if (status == OK) {
        if (string_append(response.body.data, output) == STRING_ERROR) {
            fprintf(stderr, "assembling response string error\n");
            return -1;
        }
    }

    // cleanup
    delete_string(&response.status_line);
    delete_string(&response.header);
    delete_string(&response.body);
    return 0;
};

//
//status line builder takes status code and response lne string to edit
int response_statusline_builder(enum STATUS_CODE s, string *status_line)
{
    if (string_append(HTTP_VERSION, status_line) == STRING_ERROR) {
        fprintf(stderr, "response head builder string error\n");
        return -1;
    }
    switch (s) {
        case OK:
            if (string_append(" 200 OK", status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
        case BAD_REQUEST:
            if (string_append(" 400 Bad Request", status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
        case FILE_NOT_FOUND:
            if (string_append(" 404 Not Found", status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
        case SERVER_ERROR:
            if (string_append(" 500 Internal Server Error", status_line) == STRING_ERROR) {
                fprintf(stderr, "response head builder string error\n");
            }
            break;
    }
    if (string_append(CRLF, status_line) == STRING_ERROR) {
        fprintf(stderr, "response head builder string error\n");
        return -1;
    }
    return 0;
}

// header crafter
int response_header_builder(string *header)
{
    int header_field_amnt = 2;
    char *header_fields[header_field_amnt];
    header_fields[0] = "Connection: close\r\n";
    header_fields[1] = "Content-Type: text/html\r\n";
    header_fields[2] = "Server: FlascServer\r\n";

    for (int i = 0; i < header_field_amnt; i++)
    {
        if (string_append(header_fields[i], header) == STRING_ERROR) {
            fprintf(stderr, "assembling response header string error\n");
            return -1;
        }
    }
    return 0;
}   

// body crafter
int response_body_builder(string *body, char *key)
{
    // assembling file path
    char pagePath[MAX_PATH];
    char *filename = node_search()
    snprintf(pagePath, sizeof(pagePath), "%s\\static\\%s", rootpath, routing_table[table_index].page.data);

    FILE *page_file = fopen(pagePath, "r");
    if (!page_file) return -1;

    fseek(page_file, 0, SEEK_END);
    long size = ftell(page_file); // determining file size
    fseek(page_file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    if (!buffer) return -1;

    size_t read_bytes = fread(buffer, 1, size, page_file);
    buffer[read_bytes] = '\0';

    if (string_append(CRLF, body) == STRING_ERROR) { // carriage return new line to indicate body
        fprintf(stderr, "response body builder string error\n");
        return -1;
    }
    if (string_append(buffer, body) == STRING_ERROR) {
        fprintf(stderr, "response body builder string error\n");
        return -1;
    }
    fclose(page_file);
    free(buffer);
    return 0;
}

enum STATUS_CODE request_header_validator(char *request_line)
{
    if (!request_line) return BAD_REQUEST;
    char *save_element, *element, *end;
    // method validation
    element = strtok_r(request_line, SP, &save_element);
    if (strcmp(element, "GET") != 0) { // expand for POST request
        return BAD_REQUEST;
    } 
    // URI validation
    element = strtok_r(NULL, SP, &save_element);
    if (element[0] != '/') {
        return BAD_REQUEST;
    }
    int uri_len = strlen(element);

    if (uri_len == 1 && element[0] == '/')


    // for (int i = 0; i < route_count; i ++)
    // {
    //     if (uri_len == 1){
    //         if (strcmp(element, routing_table[i].URI.data) == 0) {
    //         *table_index = i;
    //         break;
    //     }
    //     } else if (uri_len > 1) {
    //         end = strrchr(element, '.'); // slicing URI by dot character
    //         if (end != NULL) {
    //             *end = '\0';
    //         }        
    //         if (strcmp(element, routing_table[i].URI.data) == 0) {
    //             *table_index = i;
    //             break;
    //         }
    //     }
    //     if (i == route_count - 1) {
    //         return FILE_NOT_FOUND;
    //     }
    // }

    // version validation
    element = strtok_r(NULL, SP, &save_element);
    if (strcmp(element, HTTP_VERSION) != 0 && strcmp(element, "HTTP/1.1") != 0 ) {
        return BAD_REQUEST;
    } 
    return OK;
}

char *get_mime()
{

}