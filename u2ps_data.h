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
	const char* base;
	const char** fonts;
} fontvariants[];

extern const struct fontrange {
	int from;
	int to;
	int fontstyle;
} fontranges[];
