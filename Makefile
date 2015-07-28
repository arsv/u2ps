include config.mk

default: u2ps psfrem

psfrem: psfrem.o psfrem_opts.o psfrem_filt.o psfrem_list.o psfrem_util.o \
	resuffix.o warn.o

u2ps: u2ps.o u2ps_opts.o u2ps_data.o u2ps_file.o u2ps_pswr.o u2ps_term.o \
	u2ps_tset.o u2ps_tcsi.o u2ps_unicode.o resuffix.o warn.o

ttf2pt42: ttf2pt42.o warn.o

%: %.o
	$(CC) -o $@ $(filter %.o,$^)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

install:
	mkdir -p $(DESTDIR)$(bindir)
	install -m 0755 u2ps $(DESTDIR)$(bindir)/u2ps
	install -m 0755 psfrem $(DESTDIR)$(bindir)/psfrem
	mkdir -p $(DESTDIR)$(basedir)
	cp -r ps/* $(DESTDIR)$(basedir)/
	mkdir -p $(DESTDIR)$(man1dir)
	install -m 0644 u2ps.1 $(DESTDIR)$(man1dir)/u2ps.1
	install -m 0644 psfrem.1 $(DESTDIR)$(man1dir)/psfrem.1

install-ttf2pt42:
	install -m 0755 ttf2pt42 $(DESTDIR)$(bindir)/ttf2pt42

clean:
	rm -f *.o

distclean: clean
	rm -f u2ps psfrem ttf2pt42 tags

# dependencies

u2ps*.o: u2ps.h
u2ps_opts.o: u2ps_data.h
