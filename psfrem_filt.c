#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "psfrem.h"
#include "warn.h"

#define BUF 1024
#define RES 100

static FILE* output;
extern struct dynlist libpath;
static struct dynlist resources;

/* This is the main non-PS part of psfrem, filtering the contents
   of inputname into outputname, adding resources, and/or
   the contents of statsname which is reduce stage output.

   Required and provided resources are inferred from DSC comments
   in the input stream and the resources themselves.

   GS include path (libpath, -Idir options) are used to locate
   embeddable resource files. */

static int good_place_for_stats(char* line, int linenum);
static void include_resource(char* restag, char* fromfile, int fromline);
static void mark_resource(char* restag, char* fromfile, int fromline);
static void include_stats(char* statsname);

void filter_embed(char* outputname, char* inputname, char* statsname)
{
	FILE* input;
	char line[BUF];

	int inputline = 0;
	int inres = 0;
	int contline = 0;
	char* rest;

	output = outputname ? fopen(outputname, "w") : stdout;
	input = inputname ? fopen(inputname, "r") : stdin;

	if(!output)
		die("Cannot create %s: %m\n", outputname);
	if(!input)
		die("Cannot open %s: %m\n", inputname);

	while(fgets(line, BUF, input))
	{
		if(!contline)
			inputline++;
		else			/* If we're still printing out a line that happened to be   */
			goto print;	/* longer than BUF, skip all checks and just print it.      */
		
		if(statsname && good_place_for_stats(line, inputline)) {
			include_stats(statsname);
			statsname = NULL;
		}

		if((rest = prefixed(line, "%%BeginResource:"))) {
			mark_resource(rest, inputname, inputline);
			inres = 1;
		} else if(prefixed(line, "%%EndResource:")) {
			inres = 0;
		} else if((rest = prefixed(line, "%%IncludeResource:"))) {
			if(inres) die("%s:%i: %%%%IncludeResource inside another resource\n",
						inputname, inputline);
			include_resource(rest, inputname, inputline);
			continue;
		}

		print: fputs(line, output);

		contline = !endswith(line, '\n');
	}
}

/* Where should we dump reduced resources?

   The natural position seems to be "after other resources but before the code".
   In an DSC-sectioned document, that's before prolog or lacking that, before
   the first page. In non-DSC-conforming document, we cannot take chances and
   must do it before the first line.

   Only the first positive return counts; filter_embed takes care not to embed
   reduced resources twice. */

int good_place_for_stats(char* line, int linenum)
{
	if(linenum == 1 && (line[0] != '%' || line[1] != '!'))
		return 1;

	if(prefixed(line, "%%BeginProlog")
	|| prefixed(line, "%%Page")
	|| prefixed(line, "%%BeginSetup")
	|| prefixed(line, "%%BeginPageSetup"))
		return 1;

	return 0;
}

void include_stats(char* statsname)
{
	FILE* stats = fopen(statsname, "r");
	char line[BUF];
	char* rest;
	int inputline = 0;
	int contline = 0;

	if(!stats)
		die("Cannot open %s: %m\n", statsname);
	while(fgets(line, BUF, stats)) {
		if(!contline) {
			inputline++;
			if((rest = prefixed(line, "%%BeginResource:"))) {
				mark_resource(rest, statsname, inputline);
			} else if((rest = prefixed(line, "%%IncludeResource:"))) {
				include_resource(rest, statsname, inputline);
				continue;
			}
		}
		fputs(line, output);
		contline = !endswith(line, '\n');
	}

	fclose(stats);
}

int split_resource_name(char* resname, char** catptr, char** resptr)
{
	const char* space = " \t\n";

	char* category = resname + strspn(resname, space);
	int catlen = strcspn(category, space);
	char* separator = category + catlen;
	char* resource = separator + strspn(separator, space);
	int reslen = strcspn(resource, space);

	if(!catlen || !reslen)
		return -1;

	*catptr = category;
	*resptr = resource;

	category[catlen] = '\0';
	resource[reslen] = '\0';

	return 0;
}

int insert_resource(const char* category, const char* resource)
{
	char* resname = strecat(category, " ", resource, NULL);
	int known;

	if(!(known = dinlist(&resources, resname)))
		dapush(&resources, resname);
	else
		free(resname);

	return known;
}

/* restag points to a line that will be printed, and split_resource_name
   modifies its argument in place, so got to make a copy here. */

void mark_resource(char* restag, char* file, int line)
{
	char* category;
	char* resource;
	char buf[strlen(restag)+1];
	strcpy(buf, restag);

	if(split_resource_name(buf, &category, &resource))
		die("%s:%i: bad resource name %s\n", file, line, restag);

	if(insert_resource(category, resource))
		die("%s:%i: duplicate resource %s %s\n", file, line, category, resource);
}

/* Category description (that is, resource "category $cat") must be included
   before any resource "$cat $res", but only once, and not for built-in PS
   categories like procset and font.

   Most definitions are trivial and could be generated on the fly, *but* gs
   needs actual files when running without embedded resources, and who knows,
   maybe non-trivial category definitions are possible as well. */

int need_category(const char* category)
{
	static const char* innate[] = { "font", "procset", "category", "encoding", NULL };
	const char** p;

	for(p = innate; *p; p++)
		if(!strcmp(*p, category))
			return 0;

	char* resname = strecat("category ", category, NULL);
	int known = dinlist(&resources, resname);
	free(resname);
	return !known;
}

void include_cat_res(char* category, char* resource, char* fromfile, int fromline);
void include_resfile(const char* category, const char* resource, FILE* resfile);

void include_resource(char* restag, char* fromfile, int fromline)
{
	char* category;
	char* resource;

	if(split_resource_name(restag, &category, &resource))
		die("%s:%i: bad resource name %s\n", fromfile, fromline, restag);

	if(insert_resource(category, resource))
		return;

	include_cat_res(category, resource, fromfile, fromline);
}

/* Now we need to include something like "procset Foo", but the thing is,
   DSC standard (or GS implementation?) makes it kinda difficult,
   because its file name is going to be ProcSet/Foo. To do that, we scan
   libpath directories and pick any that matches the category case-insensitively.

   The situation is even worse with fonts, because the file containing "font Some-Name"
   may in fact be named fonts/sn1020.pfb and may not be embeddable in that form.
   Reduce solves that problem by dumping font data from a running gs instance,
   but only for the fonts it was allowed to process.

   For now, we assume that any font F psfrem is allowed to embed as a resource
   is contained in fonts/F.pfa, and it is ok to drop anything else.
   This is enough not handle notdef, the only font u2ps needs that should not
   be reduced. */

FILE* try_font(const char* dir, const char* fontname);
FILE* try_other(const char* dir, const char* category, const char* resource);

void include_cat_res(char* category, char* resource, char* fromfile, int fromline)
{
	extern FILE* output;
	FILE* resfile;
	int isfont = !strcmp(category, "font");
	char** d;

	if(!libpath.list)
		return NULL;

	for(d = libpath.list; *d; d++) {
		if(isfont)
			resfile = try_font(*d, resource);
		else
			resfile = try_other(*d, category, resource);
		if(resfile)
			break;
	}

	if(!resfile) {
		warn("%s:%i: resource %s %s not found\n", fromfile, fromline, category, resource);
		fprintf(output, "%%%%DocumentNeedsResources: %s %s\n", category, resource);
	} else {
		include_resfile(category, resource, resfile);
		fclose(resfile);
	}
}

FILE* try_font(const char* dir, const char* fontname)
{
	char* path = strecat(dir, "/fonts/", fontname, ".pfa", NULL);
	FILE* res = fopen(path, "r");
	free(path);
	return res;
}

FILE* try_other(const char* dir, const char* category, const char* resource)
{
	FILE* res = NULL;
	DIR* dh;
	struct dirent* de;
	int catlen = strlen(category);
	char* path = strecat(dir, "/", category, "/", resource, NULL);
	char* catptr = path + strlen(dir) + 1;

	if(!(dh = opendir(dir)))
		goto out;

	while((de = readdir(dh))) {
		if(strlen(de->d_name) != catlen)
			continue;
		if(strcasecmp(de->d_name, category))
			continue;
		memcpy(catptr, de->d_name, catlen);
		if((res = fopen(path, "r")))
			break;
	};

out:	free(path);
	return res;
}

void include_resfile(const char* category, const char* resource, FILE* resfile)
{
	extern FILE* output;
	char line[BUF];
	int contline = 0;
	int skip = 0;

	fprintf(output, "%%%%BeingResource: %s %s\n", category, resource);
	while(fgets(line, BUF, resfile)) {
		if(!contline)
			skip = (line[0] == '%' && (line[1] == '%' || line[1] == '!'));
		if(!skip)
			fputs(line, output);
		contline = !endswith(line, '\n');
	}
	fprintf(output, "%%%%EndResource\n");
}

