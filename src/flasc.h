typedef struct {
    char *data;
    int len;
    int cap;
} string;

int server(void);

string *string_builder(void);