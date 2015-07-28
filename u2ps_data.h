extern const struct papersize {
	char* name;
	short pw, ph;	
	short mv, mh;
} papersize[];

extern const struct fontaspect {
	char* name;
	int aspect;	
} fontaspects[];

extern const struct fontvariant {
	char* base;	/* REGULAR */
	char* font[3];	/* BOLD, ITALIC, BOLDITALIC */
} fontvariants[];
