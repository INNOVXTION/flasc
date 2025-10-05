typedef struct {
    char *data;
    int len;
    int cap;
} string;

int server(void);

int string_builder(int cap, string* new_string);
int string_append(char *text, string *string);
int string_slicer(int low, int high, string *input_string, string *slice);
