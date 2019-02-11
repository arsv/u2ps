#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "u2ps.h"
#include "warn.h"

char* preptemplate(const char* template)
{
	char* dir = getenv("TMPDIR");
	if(!dir) dir = "/tmp";

	int templen = strlen(template);
	int dirlen = strlen(dir);
	char* out = malloc(templen + dirlen + 2);

	if(!out) die("Cannot allocate memory: %m\n");

	strncpy(out, dir, dirlen);
	out[dirlen] = '/';
	strncpy(out + dirlen + 1, template, templen);
	out[dirlen + templen + 1] = '\0';

	return out;
}

FILE* fmkstemps(char* template, int suffixlen)
{
	int fd;

	if((fd = mkstemps(template, suffixlen)) < 0)
		return NULL;

	return fdopen(fd, "w");
}
