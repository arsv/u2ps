include ../config.mk

CFLAGS += -I.. -Dstatic= -ffunction-sections
LDFLAGS += -Wl,-z,muldefs

all: test_strecat test_deutf test_takecsi test_u2opts

clean:
	rm -f *.o

test_%.o: test_%.c
	$(CC) $(CFLAGS) -o $@ -c $<

u2ps_%.o: ../u2ps_%.c
	$(CC) $(CFLAGS) -o $@ -c $<

psfrem_%.o: ../psfrem_%.c
	$(CC) $(CFLAGS) -o $@ -c $<

psfrem: ../psfrem
	cp $< $@

../psfrem:
	$(MAKE) -C .. $(notdir $@)

warn.o: ../warn.c
	$(CC) $(CFLAGS) -o $@ -c $<

test_%: test_%.o
	$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)

test_deutf: test_deutf.o u2ps_unicode.o

test_takecsi: test_takecsi.o globals.o u2ps_term.o u2ps_tset.o u2ps_tcsi.o u2ps_pswr.o u2ps_unicode.o

test_strecat: test_strecat.o psfrem_util.o

test_u2opts: test_u2opts.o u2ps_opts.o warn.o
