#include "flasc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#define ROUTE_COUNT 5
#define FILE_LINE_BUFFER 512
#define KEY_BUFFER 128
#define VALUE_BUFFER 128

struct route routing_table[ROUTE_COUNT];
char port[5];
char rootpath[MAX_PATH];

int route_init();

int main() {
    if (route_init() != 0) {
        fprintf(stderr, "config error, shutting down...");
        exit(1);
    }
    server();
    return 0;
}

// create route tabele
int route_init()
{
    // NULL → get path of the current executable
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
    char *start, *end;
    int table_index = 0;
    char result[100];

    while (fgets(line, FILE_LINE_BUFFER - 1, config_file)) {

        if (table_index > ROUTE_COUNT -1) {
            fprintf(stderr, "ERROR, too many pages! maximum: 5\n");
            fprintf(stderr, "FILE READ ERROR\n");
            free(line);
            free(key);
            free(value);
            return -1;
        }
        // Skip comments and empty lines
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
                if (string_append(result, &(routing_table[table_index].URI)) == STRING_ERROR) {
                    fprintf(stderr, "CONFIG STRING URI APPEND ERROR\n");
                };
                if (string_append(value, &(routing_table[table_index].page)) == STRING_ERROR) {
                    fprintf(stderr, "CONFIG STRING PAGE APPEND ERROR\n");
                };
                table_index++;
                fprintf(stderr, "rout added: %s for %s\n", result, value);
                continue;
            } else {
                printf("CONFIG FILE ERROR\n");
            }
        }
        else
        {
            fprintf(stderr, "FILE READ ERROR\n");
            free(line);
            free(key);
            free(value);
            return -1;
        }
    }
    fprintf(stderr, "config file successfully read!\n");
    free(line);
    free(key);
    free(value);
    return 0;
}


// shut down gracefully