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

   GS include path (libpath, -Idir options) are used to locate
   embeddable resource files. */

static void include_resource(char* restag, char* fromfile, int fromline);
static void mark_resource(char* restag, char* fromfile, int fromline);
static void include_stats(char* statsname);

/* The main loop handles IncludeResource directives, tracks resources
   already present in the file, and decides where to put stats.

   In a DSC-sectioned document, stats go at the top of the Prolog section.
   The document may not be DSC-sectioned however, or may lack Prolog for some
   reason, in which case we react to comments that should definitely come after
   the included resources. */

void filter_embed(char* outputname, char* inputname, char* statsname)
{
	FILE* input;
	char line[BUF];

	int isdsc;
	int inputline = 0;
	int statsline = 0;
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
		if(!inputline)
			isdsc = prefixed(line, "%!") ? 1 : 0;
		if(!contline)
			inputline++;
		else			/* If we're still printing out a line that happened to be   */
			goto print;	/* longer than BUF, skip all checks and just print it.      */
		
		if(!statsname)
			; /* nothing to include, or included already */
		else if(inputline == 1 && !isdsc)
			statsline = inputline;
		else if(prefixed(line, "%%BeginProlog"))
			statsline = inputline + 1;
		else if(prefixed(line, "%%Begin") || prefixed(line, "%%Page:"))
			statsline = inputline;
		if(statsname && inputline == statsline)
			include_stats(statsname), statsname = NULL;

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

/* There are three very similar file-filtering loops here, one in filter_embed,
   one here and yet another one in include_resfile. They do almost the same
   thing, with only slight variations over which DSC directives are handled,
   but factoring out the common part would only complicate the code. */

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

int push_resource(const char* category, const char* resource)
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

	if(push_resource(category, resource))
		die("%s:%i: duplicate resource %s %s\n", file, line, category, resource);
}

void include_cat_res(char* category, char* resource, char* fromfile, int fromline);
void include_resfile(const char* category, const char* resource, FILE* resfile);

void include_resource(char* restag, char* fromfile, int fromline)
{
	char* category;
	char* resource;

	if(split_resource_name(restag, &category, &resource))
		die("%s:%i: bad resource name %s\n", fromfile, fromline, restag);

	if(push_resource(category, resource))
		return;

	include_cat_res(category, resource, fromfile, fromline);
}

/* We need to include something like "procset Foo", but the DSC standard
   (or its GS implementation?) makes it somewhat difficult than it should be
   because its file name is going to be ProcSet/Foo; note procset vs ProcSet.
   To resolve this, we scan libpath directories and pick any that matches
   the category case-insensitively.

   The situation is even worse with fonts, because the file containing
   "font Some-Name" may in fact be named fonts/sn1020.pfb and may not be
   embeddable in that form. Font-reducing code solves this problem by dumping
   font data directly from a running gs instance, but not all fonts get passed
   through that.

   So for now, we assume that any font F psfrem is allowed to embed as
   a resource is contained in a file named fonts/F.pfa, and it is ok to drop
   any fonts that have no corresponding file. This is enough not handle notdef,
   the only font u2ps needs that should not be reduced. */

FILE* try_font(const char* dir, const char* fontname);
FILE* try_other(const char* dir, const char* category, const char* resource);

void include_cat_res(char* category, char* resource, char* fromfile, int fromline)
{
	extern FILE* output;
	FILE* resfile;
	int isfont = !strcmp(category, "font");
	char** d;

	if(!libpath.list)
		return;

	for(d = libpath.list; *d; d++) {
		if(isfont)
			resfile = try_font(*d, resource);
		else
			resfile = try_other(*d, category, resource);
		if(resfile)
			break;
	}

	if(!resfile) {
		warn("%s:%i: resource %s %s not found\n",
		             fromfile, fromline, category, resource);
		fprintf(output, "%%%%DocumentNeedsResources: %s %s\n",
		             category, resource);
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

