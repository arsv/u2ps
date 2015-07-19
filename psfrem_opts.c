#include <stdlib.h>
#include <string.h>
#include "warn.h"
#include "resuffix.h"
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

int keeptemp = 0;
int doreduce = 0;
int allfonts = 0;

static void exclude_corefonts(void);

void handle_options(int argc, char** argv)
{
	int i;
	char* arg;
	char* val;

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
			case 'r': doreduce = 1; break;
			case 'A': allfonts = 1; break;

			default:
				die("psfrem: unknown option -%c\n", arg[1]);
		}
	}

out:	if(i < argc)
		inputname = argv[i++];
	else if(doreduce)
		die("psfrem: input file name required\n");

	if(i < argc)
		outputname = argv[i++];
	else
		outputname = NULL;

	if(i < argc)
		die("psfrem: too many arguments\n");

	/* This is not exactly correct, especially the input name part,
	   but it works well enough for u2ps so why bother with proper tmpnames. */
	if(doreduce)
		statsname = resuffix(outputname ? outputname : inputname, ".ps", ".sts");

	if(!allfonts)
		exclude_corefonts();
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
