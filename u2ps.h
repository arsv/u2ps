#define NONE		0
#define STANDARD45	1
#define NONSTANDARD	2

#define NO		0
#define YES		1

#define FILENAME ((void*) 1)
#define PAGENO   ((void*) 2)

typedef unsigned char bool;

struct genopts {
	int startline;
	bool landscape;
	bool skipansi;
	bool wrap;
	bool mark;
	bool inverse;
	bool linenum;
	bool bookish;

	char* title;

	int cols;
	int rows;
	int tabstop;
};

struct headings {
	bool any;
	char *hl, *hc, *hr;	/* header/footer left/center/right */
	char *fl, *fc, *fr;
};

struct runopts {
	bool skipfonts;
	bool embedstdfonts;
	bool skipnotdef;
	bool skipfrem;
	bool stdout;
	bool skipunlink;
};

struct pagelayout {
	int pw, ph;		/* paper size, non-rotated even in landscape mode */
	int mt, mb, ml, mr;
};

struct font {
	int size;
	char* name;
};

struct fonts {
	struct font text;
	struct font head;
	struct font line;
};

struct term  {
	int hardline;
	int softline;
	int softcol;
};
