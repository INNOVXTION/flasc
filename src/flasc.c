#include "flasc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#define ROUTE_MAX 5
#define FILE_LINE_BUFFER 512
#define KEY_BUFFER 128
#define VALUE_BUFFER 128

struct route routing_table[ROUTE_MAX];
char port[5];
char rootpath[MAX_PATH];

int route_count = 0;
const int route_table_URI_buffer = 128;
const int route_table_page_buffer = 128;

int route_init();

int main() {
    if (route_init() != 0) {
        fprintf(stderr, "config error, shutting down...");
        exit(1);
    }
    server();
    return 0;
}

// create routing table
int route_init()
{
    // get path of flasc.exe
    DWORD length = GetModuleFileNameA(NULL, rootpath, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        fprintf(stderr, "Error getting executable path\n");
        return -1;
    }

    // truncating path string
    char *lastSlash = strrchr(rootpath, '\\');
    if (lastSlash) {
        *lastSlash = '\0'; // terminate the string to keep only the directory
    }

    char configPath[MAX_PATH];
    snprintf(configPath, sizeof(configPath), "%s\\config.ini", rootpath);

    printf("Executable directory: %s\n", rootpath);

    // reading config file
    FILE *config_file = fopen(configPath, "r");
    
    if (!config_file) return -1;

    char *line = malloc(FILE_LINE_BUFFER * sizeof(char));
    char *key = malloc(KEY_BUFFER * sizeof(char));
    char *value = malloc(VALUE_BUFFER * sizeof(char));
    char *result = malloc (100 * sizeof(char));
    char *start, *end;

    while (fgets(line, FILE_LINE_BUFFER - 1, config_file)) {

        // skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;

        if (sscanf(line, "%63[^=]=%63s", key, value) == 2) {
            // printf("key: %s, value: %s\n", key, value);
            // gets port
            if (strcmp(key, "port") == 0)
            {
                fprintf(stderr, "port registered: %s\n", value);
                memcpy(port, value, 5 * sizeof(char));
                continue;
            }
            // tokenizing route key and add to table
            start = strchr(key, '"');  
            end   = strrchr(key, '"');
            if (start && end && start != end) {
                size_t len = end - start - 1;    
                strncpy(result, start + 1, len); 
                result[len] = '\0';
                if (string_builder(route_table_URI_buffer, &(routing_table[route_count].URI)) == STRING_ERROR) {
                    fprintf(stderr, "CONFIG STRING URI BUILD ERROR\n");
                };
                if (string_append(result, &(routing_table[route_count].URI)) == STRING_ERROR) {
                    fprintf(stderr, "CONFIG STRING URI APPEND ERROR\n");
                };
                if (string_builder(route_table_page_buffer, &(routing_table[route_count].page)) == STRING_ERROR) {
                    fprintf(stderr, "CONFIG STRING URI BUILD ERROR\n");
                };
                if (string_append(value, &(routing_table[route_count].page)) == STRING_ERROR) {
                    fprintf(stderr, "CONFIG STRING PAGE APPEND ERROR\n");
                };
                route_count++;
                fprintf(stderr, "rout added: %s for %s\n", result, value);
                continue;
            } else {
                printf("CONFIG FILE ERROR\n");
            }
        }
        else
        {
            fprintf(stderr, "FILE READ ERROR\n");
            fclose(config_file);
            free(line);
            free(key);
            free(value);
            free(result);
            return -1;
        }
    }
    fprintf(stderr, "config file successfully read!\n");
    fclose(config_file);
    free(line);
    free(key);
    free(value);
    free(result);
    return 0;
}
