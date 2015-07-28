#include "u2ps.h"

extern struct genopts genopts;
extern struct fonts fonts;
extern struct pagelayout pagelayout;

extern void psnl(int blanks);
extern void psline(const char* line, ...);

static void findfont(const char* var, const struct font* f);

void put_global_setup(void)
{
	psline("%%%%BeginSetup\n");
	psline("/Lga dup /Unidata findresource def\n");
	psline("/unifont/ProcSet findresource { def } forall\n");
	psline("/uniterm/ProcSet findresource { def } forall\n");
	psnl(1);

	int fs = fonts.basesize;
	int fa = fonts.aspect;
	psline("/em %i cpt def  	%% terminal grid x-step\n", fs*fa/1000);
	psline("/ex %i cpt def  	%% terminal grid y-step\n", fs);
	psline("/tabstop %i def\n", genopts.tabstop);
	psnl(1);

	findfont("fh", &fonts.head);
	findfont("fl", &fonts.line);
	findfont("fR", &fonts.text[REGULAR]);
	findfont("fB", &fonts.text[BOLD]);
	findfont("fI", &fonts.text[ITALIC]);
	findfont("fO", &fonts.text[BOLDITALIC]);
	psnl(1);

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
	psline("/term-yh term-yt headsep add %i cpt .2 mul add def\n", fonts.head.size);
	psline("/term-yf term-yb headsep sub %i cpt .8 mul sub def\n", fonts.head.size);
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

void findfont(const char* name, const struct font* f)
{
	if(f->xscale && f->xscale != 1000)
		psline("/%s /%s %i cpt %.1f mil fontcmd def\n",
				name, f->name, f->size, f->xscale);
	else
		psline("/%s /%s %i cpt fontcmd def\n",
				name, f->name, f->size);
}
