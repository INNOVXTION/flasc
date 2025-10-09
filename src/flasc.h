#ifndef FLASC_H
#define FLASC_H
#define STRING_ERROR -1
#define ROUTE_COUNT 5


typedef struct {
    char *data;
    int len;
    int cap;
} string;

struct route {
    string URI;
    string page;
};

struct node {
    char *key;
    char *value;
    struct node *next;
};

struct hashtable {
    int len;
    int cap;
    struct node **array;
};

extern struct hashtable ht;
extern char port[];
extern char rootpath[];

int server(void);

int string_builder(int cap, string* new_string);
int string_append(char *text, string *string);
int string_slicer(int low, int high, string *input_string, string *slice);
int delete_string(string *string);

int http_handler(string *request, string *output);

int hashtable_set (struct hashtable *ht, int cap);
int node_append(char *key, char *value, struct hashtable *ht);
char *node_search(char *key, struct hashtable *ht);
void hashtable_delete(struct hashtable *ht);

#endif