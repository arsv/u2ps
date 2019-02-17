#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "u2ps.h"
#include "warn.h"
#include "u2ps_data.h"

char* inputname = NULL;
char* outputname = NULL;
char* tmpoutname = NULL;

char** passopts = { NULL };
int passnum = 0;

int verbose = 0;

struct genopts genopts = {
	.startline = 1,
	.landscape = NO,
	.cols = 0,
	.rows = 0,
	.tabstop = 8
};

struct runopts runopts;

struct pagelayout pagelayout = {
	.pw = 595, .ph = 842,       /* A4 */
	.mt = 55,  .ml = 57.5,      /* some reasonable margins */
	.mb = 55,  .mr = 57.5
};

int auxsize; /* centipoints */
const char* auxfont = "Times-Roman";

int fontsize; /* centipoints */
int fontaspect;
struct font fonts[nFONTS];

struct headings headings;

static bool halfchar = NO;
static bool widechar = NO;

static void die_print_usage(void);
static int handle_longopt(int argc, char** argv, int i);
static int handle_shortopt(int argc, char** argv, int i);
static void set_derivative_parameters(void);

void handle_args(int argc, char** argv)
{
	int i = 1;
	int (*handler)(int argc, char** argv, int i);

	if(argc < 2)
		die_print_usage();

	passopts = malloc(argc*sizeof(char*));
	memset(passopts, 0, argc*sizeof(char*));
	memset(&headings, 0, sizeof(headings));

	while(i < argc) {
		if(argv[i][0] != '-')
			break;
		else if(!argv[i][1])
			{ i++; goto LO; }
		else if(argv[i][1] != '-')
			handler = handle_shortopt;
		else if(!argv[i][2])
			{ i++; goto LI; }
		else
			handler = handle_longopt;

		i += handler(argc, argv, i) ? 2 : 1;
	}

LI:	if(i < argc)
		inputname = argv[i++];
LO:	if(i < argc)
		outputname = argv[i++];
	if(i < argc)
		die("Too many arguments\n");

	if(runopts.tostdout && !inputname)
		die("Either use - or specify input file name with -o\n");
	if(runopts.tostdout && outputname)
		die("Using -o with output file name makes no sense\n");

	set_derivative_parameters();
}

void die_print_usage(void)
{
	printf("Usage: u2ps [options] input [output]\n");
	exit(0);
}

#define BOOL    0
#define INT     1
#define STRING  2
#define FUNC    3
#define PASS    4
#define FSIZE   5

static void set_paper(char* opt);
static void set_margins(char* opt);
static void set_empty_headings(void);
static void add_passopt(char* opt);
static void set_font(char* opt);

struct option {
	short shortopt;
	char* longopt;
	short type;
	void* addr;
} optlist[] = {
	{ 'T', "tab",           INT,    &genopts.tabstop    },
	{ 'a', "noansi",        BOOL,   &genopts.skipansi   },
	{ '1', "wide",          BOOL,   &widechar           },
	{ '2', "half",          BOOL,   &halfchar           },
	{ 'P', "paper",         FUNC,   set_paper           },
	{ 'M', "margins",       FUNC,   set_margins         },
	{ 'r', "landscape",     BOOL,   &genopts.landscape  },
	{ 'C', "columns",       INT,    &genopts.cols       },
	{ 'L', "lines",         INT,    &genopts.rows       },
	{ 's', "size",          FSIZE,  &fontsize           },
	{ '-', "font-size",     FSIZE,  &fontsize           },
	{ 'f', "font",          FUNC,   set_font            },
	{ '-', "aux-font",      STRING, &auxfont            },
	{ '-', "aux-size",      FSIZE,  &auxsize            },
	{ 'o', "stdout",        BOOL,   &runopts.tostdout   },
	{ 'w', "wrap",          BOOL,   &genopts.wrap       },
	{ 'm', "mark",          BOOL,   &genopts.mark       },
	{ 'i', "inverse",       BOOL,   &genopts.inverse    },
	{ 'l', "numbers",       BOOL,   &genopts.linenum    },
	{ '-', "startline",     INT,    &genopts.startline  },
	{ 'b', "bookish",       BOOL,   &genopts.bookish    },
	{ 't', "title",         STRING, &genopts.title      },
	{ 'H', "noheadings",    FUNC,   set_empty_headings  },
	{ '-', "header-left",   STRING, &headings.hl        },
	{ '-', "header",        STRING, &headings.hc        },
	{ '-', "header-right",  STRING, &headings.hr        },
	{ '-', "footer-left",   STRING, &headings.fl        },
	{ '-', "footer",        STRING, &headings.fc        },
	{ '-', "footer-right",  STRING, &headings.fr        },
	{ 'R', "noreduce",      BOOL,   &runopts.noreduce   },
	{ 'A', "allfonts",      BOOL,   &runopts.allfonts   },
	{ 'E', "noembed",       BOOL,   &runopts.skipfrem   },
	{ 'k', "keep",          BOOL,   &runopts.keeptemp   },
	{ 'd', NULL,            PASS,   add_passopt         },
	{ 'I', NULL,            PASS,   add_passopt         },
	{ 'v', "verbose",       BOOL,   &verbose            },
	{  0  }
};

#define GOT_THIS_OPT_ONLY 0
#define GOT_NEXT_ARG_TOO  1

static void handle_opt(struct option* opt, char* arg);

int handle_longopt(int argc, char** argv, int i)
{
	struct option* opt;
	char* arg = NULL;
	int ret = GOT_THIS_OPT_ONLY;

	if((arg = strpbrk(argv[i], "=")))
		*(arg++) = '\0';

	for(opt = optlist; opt->shortopt; opt++)
		if(!opt->longopt)
			continue;
		else if(!strcmp(opt->longopt, argv[i] + 2))
			break;
	if(!opt->shortopt)
		die("Unknown option %s\n", argv[i]);

	if(opt->type != BOOL && !arg) {
		if(++i >= argc)
			die("Argument required for %s\n", argv[i]);
		arg = argv[i];
		ret = GOT_NEXT_ARG_TOO;
	}

	handle_opt(opt, arg);

	return ret;
}

int handle_shortopt(int argc, char** argv, int i)
{
	struct option* opt;
	char* arg = NULL;
	int ret = GOT_THIS_OPT_ONLY;
	char* p = argv[i] + 1;

	do {
		for(opt = optlist; opt->shortopt; opt++)
			if(*p == opt->shortopt)
				break;
		if(!opt->shortopt)
			die("Unknown option -%c\n", *p);

		if(opt->type == PASS) {
			if(p > argv[i] + 1)
				die("Cannot mix -%c with other options in %s\n", *p, argv[i]);
			if(!*(p+1))
				die("Non-spaced argument required for -%c\n", *p);
			arg = argv[i];
		} else if(opt->type != BOOL) {
			if(*(p + 1)) {
				arg = p + 1;
			} else if(++i < argc) {
				arg = argv[i];
				ret = GOT_NEXT_ARG_TOO;
			} else {
				die("Argument required for -%c\n", *p);
			}
		}

		handle_opt(opt, arg);

	} while(opt->type == BOOL && *(++p));

	return ret;
}

void handle_opt(struct option* opt, char* arg)
{
	switch(opt->type) {
		case BOOL:
			*((bool*) opt->addr) = 1;
			break;
		case INT:
			*((int*) opt->addr) = atoi(arg);
			break;
		case STRING:
			*((char**) opt->addr) = arg;
			break;
		case PASS:
		case FUNC:
			((void (*)(char*)) opt->addr)(arg);
			break;
		case FSIZE:
			*((int*) opt->addr) = 100*atoi(arg);
			break;
	}
}

/* Paper size is either a name like "a4", or numeric W:H dimensions. */

void set_paper(char* opt)
{
	char* sep = strpbrk(opt, ":");

	if(sep) {
		*sep++ = '\0';
		pagelayout.pw = atoi(opt);
		pagelayout.ph = atoi(sep);
		pagelayout.sset = 1;
		pagelayout.paper = "custom";
	} else {
		pagelayout.paper = opt;
	}
}

/* Colon-separated CSS-style margins. */

void set_margins(char* opt)
{
	char* l1 = strsep(&opt, ":");
	char* l2 = strsep(&opt, ":");
	char* l3 = strsep(&opt, ":");
	char* l4 = opt;

	pagelayout.mt = l1 ? atoi(l1) : 0;
	pagelayout.mr = l2 ? atoi(l2) : pagelayout.mt;
	pagelayout.mb = l3 ? atoi(l3) : pagelayout.mt;
	pagelayout.ml = l4 ? atoi(l4) : pagelayout.mr;

	pagelayout.mset = 1;
}

void set_empty_headings(void)
{
	headings.any = YES;
}

void add_passopt(char* opt)
{
	/* passopts is argc-long, and at most all of argv[1:] can be
	   pushed there, so there is no need for bound checks here. */
	passopts[passnum++] = opt;
	passopts[passnum] = NULL;
}

/* Fonts are specified as either fontsets or explicit keyed fonts:

         f FreeMono             sets regular, bold, italic and bolditalic
                                using fontvariant[] table
         fI:Courier-Oblique     sets italic (I) font only

   When setting several fontsets, only the entries not set by that time
   are filled. */

static void set_font_keyed(const char* opt);
static void set_font_named(const char* opt);

void set_font(char* opt)
{
	if(opt[1] == ':')
		set_font_keyed(opt);
	else
		set_font_named(opt);
}

struct font* get_keyed_font(const char* opt)
{
	const char* key;

	for(key = fontkeys; *key; key++)
		if(opt[0] == *key)
			break;
	if(!key)
		die("unknown font key: %s\n", opt);

	return &fonts[key - fontkeys];
}

/* Direct font setting: -fR:FontName */

void set_font_keyed(const char* opt)
{
	const char* name = opt + 2;
	struct font* f = get_keyed_font(opt);
	f->name = name;
}

/* Predefined fontsets: -f FontsetName.
   Fontsets are looked up in fontvariants[], and all definitions
   found there are applied in the same way set_font_keyed works.

   One special case is when non-regular font name matches that
   of the regular font. In this case, we skip the setting,
   allowing the term to fall back to fR. This is needed to avoid
   extensive font switching when e.g. Tlwg (which defines both
   regular *and* Thai ranges) is used as the primary font set. */

void set_font_named(const char* opt)
{
	const struct fontvariant* v;

	for(v = fontvariants; v->base; v++)
		if(!strcmp(v->base, opt))
			break;
	if(!v->base)
		die("unknown font set: %s\n", opt);

	const char** s;
	bool setsomething = NO;
	struct font* regular = &fonts[REGULAR];
	for(s = v->fonts; *s; s++) {
		struct font* f = get_keyed_font(*s);
		if(f->name)
			continue;
		if(f != regular && !strcmp(*s + 2, regular->name))
			continue;
		setsomething = YES;
		f->name = *s + 2;
	} if(!setsomething)
		warn("fontset %s unused\n", opt);
}

/* Font and terminal size settings are heavily interconnected: we need
   to know primary font aspect ratio before deciding terminal size,
   terminal size may affect (set) primary font size, which in turn affects
   sizes of all other fonts. Because of this, fonts are set up in two steps,
   with terminal size selection between them. */

static void set_font_aspects(void);
static void set_font_sizes(void);
static void set_termfontsize(int tbw, int tbh);

static int got_headings_set(void)
{
	int size = sizeof(headings);
	char zero[size];

	memset(zero, 0, size);

	return memcmp(&headings, zero, size);
}

static void set_page_layout(struct pagelayout* pl)
{
	const struct papersize* p;
	char* paper = pl->paper;

	if(pl->sset)
		return;

	if(!paper)
		paper = getenv("PAPER");
	if(!paper)
		paper = "a4";

	for(p = papersize; p->name; p++)
		if(!strcmp(p->name, paper))
			break;
	if(!p->name)
		die("Unknown paper size %s\n", paper);

	pl->paper = paper;
	/* page size */
	pl->pw = p->pw;
	pl->ph = p->ph;
	/* margins; defaults are always symmetric */
	int mv = p->mv;
	int mh = p->mh;

	if(!mv) mv = p->pw / 10;
	if(!mh) mh = mv;

	pl->mt = pl->mb = mv;
	pl->ml = pl->mr = mh;
}

void set_derivative_parameters()
{
	struct pagelayout* pl = &pagelayout;

	set_page_layout(pl);

	int tbw = pl->pw - pl->ml - pl->mr;
	int tbh = pl->ph - pl->mt - pl->mb;

	if(halfchar && widechar)
		die("Cannot use -1 and -2 at the same time\n");
	else if(halfchar)
		fontaspect = 500;
	else if(widechar)
		fontaspect = 1000;

	if(!fonts[REGULAR].name)
		set_font_named("FreeMono");

	set_font_aspects();

	if(genopts.landscape)
		set_termfontsize(tbh, tbw);
	else
		set_termfontsize(tbw, tbh);

	set_font_sizes();

	if(!got_headings_set()) {
		headings.hr = "#";
		headings.hl = "@";
	}

	if(genopts.mark)
		genopts.wrap = YES;
}

/* The following tries to set some consistent values for the primary
   font size, the number of lines and the number of columns for wrapping.
   Any of the three can be set by the user, and by this point bounding box
   (paper size minus margins) is known as well. */

void set_termfontsize(int tw, int th)
{
	int fs = fontsize;
	int fa = fontaspect;

	int cols = genopts.cols;
	int rows = genopts.rows;

	if(!fs && !cols && !rows)
		fs = 1000; /* in cpt; 10pt */

	int fsc = cols ? 100*tw/cols*1000/fa : 0;
	int fsr = rows ? 100*th/rows : 0;

	if(fsc && fsc < fsr) fsr = 0;
	if(fsr && fsr < fsc) fsc = 0;

	if(fsr) fs = fsr;
	if(fsc) fs = fsc;

	genopts.cols = 100*tw/fs*1000/fa;
	genopts.rows = 100*th/fs;
	fontsize = fs;

	if(!auxsize)
		auxsize = 8*fs/10;

	if(verbose)
		warn("Terminal area %ix%ipt, final font size %icpt, %i cols %i rows\n",
			tw, th, fontsize, genopts.cols, genopts.rows);
}

static int font_name_match(const char* key, const char* font)
{
	int lk = strlen(key);
	int lf = strlen(font);

	if(lf < lk)
		return 0;
	if(strncmp(key, font, lk))
		return 0;

	return (!font[lk] || font[lk] == '-');
}

void set_font_aspects(void)
{
	int i;
	struct font* fi;
	const struct fontaspect* a;

	for(i = 0; i < nFONTS; i++) {
		fi = &fonts[i];

		if(!fi->name || fi->aspect)
			continue;
		for(a = fontaspects; a->name; a++)
			if(font_name_match(a->name, fi->name))
				break;
		fi->aspect = a->aspect;
	}

	if(!fontaspect)
		fontaspect = fonts[0].aspect;
}

void set_font_sizes(void)
{
	int i;
	struct font* fi;

	for(i = 0; i < nFONTS; i++) {
		fi = &fonts[i];

		if(!fi->name)
			continue;
		if(!fi->aspect)
			continue;

		fi->xscale = 1000*fontaspect/fi->aspect;

		if(verbose)
			warn("Font %c aspect %icpt xscale %imil name %s\n",
				fontkeys[i], fi->aspect, fi->xscale, fi->name);
	}
}
