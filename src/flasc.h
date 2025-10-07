#ifndef FLASC_H
#define FLASC_H
#define STRING_ERROR -1
#define response_status_line_cap 1024 // bytes
#define response_body_cap 2048 // bytes

typedef struct {
    char *data;
    int len;
    int cap;
} string;

struct route {
    string URI;
    string page;
};

extern struct route routing_table[];
extern char port[];
extern char rootpath[];

int server(void);

int string_builder(int cap, string* new_string);
int string_append(char *text, string *string);
int string_slicer(int low, int high, string *input_string, string *slice);
int delete_string(string *string);

int http_handler(string *request, string *output);

#endif