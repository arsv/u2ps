#include <time.h>
#include "u2ps.h"
#include "warn.h"

/* PostScript prologue does not depend on the terminal state,
   only on the command line options, so it is kept here and out
   of _term* files.

   The functions are still called from _term.c, since some terminal
   setup must be done before starting and finishing the output file. */

static void put_ps_prolog(void);
static void put_ps_setup(void);
static void findallfonts(void);

void put_ps_init(void)
{
	time_t now = time(NULL);

	psline("%%!PS-Adobe-2.0\n");
	psline("%%%%BoundingBox: 0 0 %i %i\n", pagelayout.pw, pagelayout.ph);
	psline("%%%%Orientation: %s\n", genopts.landscape ? "Landscape" : "Portrait");
	if(genopts.title)
		psline("%%%%Title: %s\n", genopts.title);
	psline("%%%%Pages: (atend)\n");
	psline("%%%%Creator: u2ps\n");
	psline("%%%%CreationDate: %s", ctime(&now)); /* ctime output includes \n! */
	psline("%%%%EndComments\n");

	put_ps_prolog();
	put_ps_setup(); };

void put_ps_prolog(void)
{
	psline("%%%%BeginProlog\n");
	psline("%%%%IncludeResource: procset gscompat\n");
	psline("%%%%IncludeResource: procset unidata\n");
	psline("%%%%IncludeResource: procset unifont\n");
	psline("%%%%IncludeResource: procset uniterm\n");
	psline("%%%%EndProlog\n");
}

void put_ps_setup(void)
{
	psline("%%%%BeginSetup\n");
	psline("/gscompat/ProcSet findresource { def } forall\n");
	psline("/unidata/ProcSet findresource { def } forall\n");
	psline("/unifont/ProcSet findresource { def } forall\n");
	psline("/uniterm/ProcSet findresource { def } forall\n");
	psnl(1);

	int fs = fontsize;
	int fa = fontaspect;
	psline("/em %i cpt def  	%% terminal grid x-step\n", fs*fa/1000);
	psline("/ex %i cpt def  	%% terminal grid y-step\n", fs);
	psline("/tabstop %i def\n", genopts.tabstop);
	psnl(1);

	psline("/auxfont /%s findfont %i cpt scalefont def\n", auxfont, auxsize);

	findallfonts();

	int landscape = genopts.landscape;
	psline("%% page size\n");
	psline("/paper-w %i def\n",  landscape ? pagelayout.ph : pagelayout.pw);
	psline("/paper-h %i def\n",  landscape ? pagelayout.pw : pagelayout.ph);
	psline("/margin-t %i def\n", landscape ? pagelayout.ml : pagelayout.mt);
	psline("/margin-r %i def\n", landscape ? pagelayout.mt : pagelayout.mr);
	psline("/margin-b %i def\n", landscape ? pagelayout.mr : pagelayout.mb);
	psline("/margin-l %i def\n", landscape ? pagelayout.mb : pagelayout.ml);
	psnl(1);

	psline("%% terminal output area corners: x left/middle/right, y top/bottom\n");
	psline("/term-xl margin-l def\n");
	psline("/term-yb margin-b def\n");
	psline("/term-xr paper-w margin-r sub def\n");
	psline("/term-yt paper-h margin-t sub def\n");
	psline("/term-xm term-xl term-xr add 2 div def		%% midpoint\n");
	psnl(1);

	psline("%% starting position on the page (line 1 col 1 baseline)\n");
	psline("/term-ox term-xl def\n");
	psline("/term-oy term-yt ex .8 mul sub def\n");
	psline("%% header/footer baselines\n");
	psline("%% font depth is assumed to be .2ex\n");

	/* if headers */
	psline("/headsep ex def\n");
	psline("/term-yh term-yt headsep add %i cpt .2 mul add def\n", auxsize);
	psline("/term-yf term-yb headsep sub %i cpt .8 mul sub def\n", auxsize);
	psnl(1);

	psline("%% base terminal colors\n");
	psline("/color-fg %s def\n", genopts.inverse ? "16#FFFFFF" : "16#000000");
	psline("/color-bg %s def\n", genopts.inverse ? "16#000000" : "16#FFFFFF");
	psline("/color-hb %s def\n", "16#AAAAAA");
	psline("/color-ln %s def\n", "16#AAAAAA");
	psnl(1);

	psline("<< /PageSize [ %i %i ] >> setpagedevice\n", pagelayout.pw, pagelayout.ph);
	psline("%%%%EndSetup\n");
}

void findallfonts(void)
{
	int i;
	char key;
	const struct font* fnt;

	for(i = 0; i < nFONTS; i++) {
		key = fontkeys[i];
		fnt = &fonts[i];

		if(!key)
			continue;
		if(!fnt->name)
			switch(i) {
				case REGULAR:
					die("Regular font not defined\n");
				case BOLD:
				case ITALIC:
				case BOLDITALIC:
					psline("/f%c /fR load def\n", key);
				default:
					continue;
			}

		if(fnt->xscale && fnt->xscale != 1000)
			psline("/f%c /%s %i cpt %i mil fontcmd def\n",
					key, fnt->name, fontsize, fnt->xscale);
		else
			psline("/f%c /%s %i cpt fontcmd def\n",
					key, fnt->name, fontsize);

	}

	psnl(1);
}

void put_ps_fini(int pages)
{
	psline("%%%%Trailer\n");
	psline("%%%%Pages: %i\n", pages);
	psline("%%%%EOF\n");
}
