#include "flasc.h"

#include <stdlib.h>
#include <string.h>

int string_builder(int cap, string* new_string)
{
    if (cap <= 0) return STRING_ERROR;
    new_string->data = malloc((cap + 1) * sizeof(char));
    if (!new_string->data) {
        return STRING_ERROR;
    }
    new_string->data[0] = '\0';
    new_string->len = 0;
    new_string->cap = cap;
    return 0;
}

// only works on initialized strings
int string_append(char *text, string *string)
{
    if (!string->data) return STRING_ERROR;
    int len = strlen(text);
    if (len + string->len > string->cap)
    {
        char *new_data = realloc(string->data, (string->len + len) * 2);
        if (!new_data) {
            return -1;
        }
        string->data = new_data;
        string->cap = (string->len + len) * 2;
    }
    memcpy(string->data + string->len, text, len);
    string->len += len;
    string->data[string->len] = '\0';
    return 0;
}

int delete_string(string *string)
{
    if (!string) return STRING_ERROR; 
    free(string->data);
    string->data = NULL;
    return 0;
}

// WIP
int string_slicer(int low, int high, string *input_string, string *slice)
{
    if (((low - high) == 0) || low > high) {
        return -1;
    }
    if (input_string->len < high || input_string->len < low) {
        return -1;
    }
}
