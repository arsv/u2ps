#include <stdlib.h>
#include <string.h>
#include "warn.h"
#include "psfrem.h"

/*

Options:
	-Idir		pass to gs
	-dKEY=val	pass to gs
	-xFont		exclude (do not embed) Font
	-aFont		include Font completely, without reduction
	-A		do embed Adobe core fonts
	-r		reduce fonts (otherwise, just embed resources)
	-k		keep temporary files
	-o		output to stdout
	-c		do not auto-include categories (XXX)

*/

char* inputname;
char* outputname;
char* statsname;

struct dynlist passopt;
struct dynlist libpath;
struct dynlist excludefonts;
struct dynlist includefonts;

int keeptemp;

#define REDUCE	(1<<0)
#define STDOUT	(1<<1)
#define ADOBE	(1<<2)

static char* resuffix(const char* name, const char* oldsuff, const char* newsuff);
static void exclude_corefonts(void);

void handle_options(int argc, char** argv)
{
	int i;
	char* arg;
	char* val;
	int flags = 0;

	for(i = 1; i < argc; i++) {
		if(*(arg = argv[i]) != '-')
			break;
		switch(arg[1]) {
			case 'x':
			case 'a':
			case 'd':
			case 'I':
				if(!*(val = arg + 2))
					die("psfrem: non-spaced value must follow -%c\n", *(arg+1));
		} switch(arg[1]) {
			case '-':
				if(arg[2])
					die("psfrem: long options are not supported\n");
				else
					i++;
				goto out;

			case 'x': dapush(&excludefonts, val); break;
			case 'a': dapush(&includefonts, val); break;
			case 'I': dapush(&libpath, arg + 2);
			case 'd': dapush(&passopt, arg); break;

			case 'k': keeptemp = 1; break;
			case 'r': flags |= REDUCE; break;
			case 'A': flags |= ADOBE; break;
			case 'o': flags |= STDOUT; break;

			default:
				die("psfrem: unknown option -%c\n", arg[1]);
		}
	}

out:	if(i < argc)
		inputname = argv[i++];
	else
		die("psfrem: input file name required\n");

	if(flags & STDOUT)
		outputname = NULL;
	else if(i < argc)
		outputname = argv[i++];
	else
		outputname = resuffix(inputname, ".ps", ".tps");

	if(i < argc)
		die("psfrem: too many arguments\n");

	if(flags & REDUCE)
		statsname = resuffix(outputname ? outputname : inputname, ".ps", ".sps");

	if(!(flags & ADOBE))
		exclude_corefonts();
}

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

/* List of fonts (PostScript Core Fonts) assumed to be present
   in any postscript interpreter.
   Unless told otherwise, psfrem will not embed these fonts. */

static char* corefonts[] = {
	"AvantGarde-Book",
	"AvantGarde-BookOblique",
	"AvantGarde-Demi",
	"AvantGarde-DemiOblique",
	"Bookman-Demi",
	"Bookman-DemiItalic",
	"Bookman-Light",
	"Bookman-LightItalic",
	"Courier-Bold",
	"Courier-BoldOblique",
	"Courier",
	"Courier-Oblique",
	"Helvetica-Bold",
	"Helvetica-BoldOblique",
	"Helvetica-NarrowBold",
	"Helvetica-NarrowBoldOblique",
	"Helvetica",
	"NewCenturySchlbk-Bold",
	"NewCenturySchlbk-BoldItalic",
	"NewCenturySchlbk-Italic",
	"NewCenturySchlbk-Roman",
	"Palatino-Bold",
	"Palatino-BoldItalic",
	"Palatino-Italic",
	"Palatino-Roman",
	"Symbol",
	"Times-Bold",
	"Times-BoldItalic",
	"Times-Italic",
	"Times-Roman",
	"ZapfChancery-MediumItalic",
	"ZapfDingbats"
};

static void exclude_corefonts(void)
{
	dappend(&excludefonts, sizeof(corefonts)/sizeof(*corefonts), corefonts);
}
