#include "flasc.h"
#include <string.h>
#include <stdio.h>

#define HTTP_VERSION "HTTP/1.0"
#define CRLF "\r\n"
#define SP " "


enum STATUS_CODE {
    OK = 200,
    BAD_REQUEST = 400,
    FILE_NOT_FOUND = 404,
    SERVER_ERROR = 500
};

enum method{
    GET,
    POST,
};

struct http_response{
    string status_line;
    string header;
    string body;
};

enum STATUS_CODE request_header_validator(char *request_line);
int response_statusline_builder(enum STATUS_CODE STATUS_CODE, string *response);
int response_header_builder(string *header);

int http_handler(string *request, string *output)
{   
    struct http_response response;
    enum STATUS_CODE status;

    // initializing response struct DONT FORGET TO FREE AFTER !
    if (string_builder(response_status_line_cap, &response.status_line) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }
    if (string_builder(response_body_cap, &response.header) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }
    if (string_builder(response_body_cap, &response.body) == STRING_ERROR) {
        fprintf(stderr, "http handler string error\n");
        return -1;
    }

    // validation
    // request line formatting
    char *save_line;
    char *token_line = strtok_r(request->data, CRLF, &save_line); //checks for formatting of first lien
    if (!token_line) {
        status = BAD_REQUEST;
        response_statusline_builder(status, &response.status_line);
        delete_string(&response.status_line);
        delete_string(&response.body);
        return 0;
    };
    // validation
    // request semantic
    status = request_header_validator(token_line);
    if (status != OK) {
        response_statusline_builder(status, &response.status_line);
        delete_string(&response.status_line);
        delete_string(&response.body);
        return 0;
    }
    
    // assembling response string
    response_statusline_builder(status, &response.status_line);
    response_header_builder(&response.header);

    if (string_append(response.status_line.data, output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string erro\n");
        return -1;
    }
    if (string_append(response.header.data, output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string erro\n");
        return -1;
    }
    // if (string_append(&response.body->data, output) == STRING_ERROR) {
    //     fprintf(stderr, "assembling response string erro\n");
    //     return -1;
    // }
    // header

    // body
    if (string_append("\r\nhello, world!", output) == STRING_ERROR) {
        fprintf(stderr, "assembling response string erro\n");
        return -1;
    }
    delete_string(&response.status_line);
    delete_string(&response.header);
    delete_string(&response.body);
    return 0;

};

//response crafter
//header, takes status code and response lne string to edit
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

    for (int i = 0; i < header_field_amnt; i++)
    {
        if (string_append(header_fields[i], header) == STRING_ERROR) {
            fprintf(stderr, "assembling response string erro\n");
            return -1;
        }
    }
    return 0;
}   

// body crafter
int response_body_builder()
{
    return 0;
}

enum STATUS_CODE request_header_validator(char *request_line)
{
    char *save_element, *element;
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
    // version validation
    element = strtok_r(NULL, SP, &save_element);
    if (strcmp(element, HTTP_VERSION) != 0 && strcmp(element, "HTTP/1.1") != 0 ) {
        return BAD_REQUEST;
    } 
    return OK;
}