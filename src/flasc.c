#include "flasc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#define ROUTE_MAX 5
#define FILE_LINE_BUFFER 512
#define KEY_BUFFER 128
#define VALUE_BUFFER 128
#define BUCKET_SIZE 100


char port[5];
char rootpath[MAX_PATH];

struct hashtable ht;

const int route_table_URI_buffer = 128;
const int route_table_page_buffer = 128;

int route_init();

int main() {
    if (route_init() != 0) {
        fprintf(stderr, "config error, shutting down...");
        exit(1);
    }
    server();
    hashtable_delete(&ht);
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
    char *result_key = malloc (100 * sizeof(char));
    char *value = malloc(VALUE_BUFFER * sizeof(char));
    char *start, *end;

    if (hashtable_set(&ht, BUCKET_SIZE) != 0) {
        fprintf(stderr, "HASH TABLE INIT ERROR\n");
    }

    while (fgets(line, FILE_LINE_BUFFER - 1, config_file)) {

        // skip comments and empty lines
        if (line[0] == '[' || line[0] == '\n') continue;

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
                strncpy(result_key, start + 1, len); 
                result_key[len] = '\0';
                if (node_append(result_key, value, &ht) != 0) {
                    fprintf(stderr, "HASH APPEND ERROR\n");
                };
                fprintf(stderr, "rout added: %s for %s\n", result_key, value);
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
            free(result_key);
            return -1;
        }
    }
    fprintf(stderr, "config file successfully read!\n");
    fclose(config_file);
    free(line);
    free(key);
    free(value);
    free(result_key);
    return 0;
}


