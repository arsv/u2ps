#include <stdlib.h>
#include <string.h>
#include "resuffix.h"

char* resuffix(const char* name, const char* oldsuff, const char* newsuff)
{
	int namelen = strlen(name);
	int oldsufflen = strlen(oldsuff);
	int newsufflen = strlen(newsuff);
	char* newname = malloc(namelen + newsufflen + 2);

	strcpy(newname, name);
	if(namelen > oldsufflen && !strcmp(newname + namelen - oldsufflen, oldsuff))
		strcpy(newname + namelen - oldsufflen, newsuff);
	else
		strcpy(newname + namelen, newsuff);

	return newname;
}
