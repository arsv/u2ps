#include <stdlib.h>
#include <string.h>
#include "psfrem.h"

#define GRAN 10

int granalign(int x) { return x + (GRAN - x % GRAN); }

void dalloc(struct dynlist* d, int spaceneeded)
{
	int spaceleft = d->len - d->ptr;
	int spacediff = spaceneeded - spaceleft;

	if(spacediff < 0)
		return;
	else
		spacediff = granalign(spacediff);

	if(!d->list)
		d->list = malloc(spacediff*sizeof(char*));
	else
		d->list = realloc(d->list, (d->len + spacediff)*sizeof(char*));

	memset(d->list + d->len, 0, spacediff*sizeof(char*));
	d->len += spacediff;
}

void dapush(struct dynlist* d, char* string)
{
	dalloc(d, 2);
	d->list[d->ptr++] = string;
	d->list[d->ptr] = NULL;
}

void dappend(struct dynlist* d, int n, char** strings)
{
	dalloc(d, n + 1);
	memcpy(d->list + d->ptr, strings, n*sizeof(char*));
	d->ptr += n;
	d->list[d->ptr] = NULL;
}

int dinlist(struct dynlist* d, char* s)
{
	char** p;

	for(p = d->list; p < d->list + d->ptr; p++)
		if(!strcmp(*p, s))
			return 1;

	return 0;
}
