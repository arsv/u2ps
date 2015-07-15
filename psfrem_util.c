#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

char* prefixed(char* string, const char* prefix)
{
	int len = strlen(prefix);
	return strncmp(string, prefix, len) ? NULL : string + len;
}

int endswith(const char* line, char c)
{
	int len = strlen(line);
	return (len && line[len-1] == c);
}

char* strecat(const char* str, ...)
{
	va_list ap;
	int len = strlen(str);
	char* p;
	char* q;
	int l;

	va_start(ap, str);
	while((p = va_arg(ap, char*)) != NULL)
		len += strlen(p);
	va_end(ap);

	char* buf = malloc(len + 1); q = buf;

	memcpy(q, str, l = strlen(str)); q += l;
	va_start(ap, str);
	while((p = va_arg(ap, char*)) != NULL) {
		memcpy(q, p, l = strlen(p)); q += l;
	};
	va_end(ap);
	*q = '\0';

	return buf;
}
