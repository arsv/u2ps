struct dynlist {
	char** list; 
	int len;
	int ptr;
};

void dalloc(struct dynlist* d, int space);
void dapush(struct dynlist* d, char* string);
void dappend(struct dynlist* d, int n, char** strings);
int dinlist(struct dynlist* d, char* s);

#define count(a) (sizeof(a)/sizeof(*a))

char* prefixed(char* string, const char* prefix);
int endswith(const char* line, char c);
char* strecat(const char* str, ...);
