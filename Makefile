CC = gcc
CFLAGS = -Wall -g

perl = /usr/bin/perl
gs = gs

bindir = /usr/bin
basedir = /usr/share/u2ps
man1dir = /usr/share/man/man1

scripts = u2ps.px psfrem.px ttf2pt42.px

all: scripts man

# make copies of *.px, setting up proper paths
scripts: $(scripts:.px=.x)

%.x: %.px relib.sed
	sed -f relib.sed $< > $@

.INTERMEDIATE: relib.sed
relib.sed: Makefile
	echo '1c#!$(perl)' > $@
	echo '/^our .GS = /s@=.*@= "$(gs)";@' >> $@
	echo '/^our .BASE = /s@=.*@= "$(basedir)";@' >> $@

install:
	mkdir -p $(DESTDIR)$(bindir)
	mkdir -p $(DESTDIR)$(basedir)
	cp -r ps $(DESTDIR)$(basedir)/
	cp -r pl $(DESTDIR)$(basedir)/
	cp paper corefonts $(DESTDIR)$(basedir)/
	install -m 0755 u2ps.x $(DESTDIR)$(bindir)/u2ps
	install -m 0755 psfrem.x $(DESTDIR)$(basedir)/psfrem.px
	mkdir -p $(DESTDIR)$(man1dir)
	-cp u2ps.1 $(DESTDIR)$(man1dir)/

install-ttf2pt42:
	install -m 0755 ttf2pt42.x $(DESTDIR)$(bindir)/ttf2pt42

clean:
	rm -f $(scripts:.px=.x)
