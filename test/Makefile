include ../config.mk

CFLAGS += -I.. -Dstatic= -ffunction-sections
LDFLAGS += -Wl,-z,muldefs

CTESTS = ct_strecat ct_deutf ct_takecsi ct_u2opts ct_fonts1 ct_fonts2
PTESTS = pt_deutf pt_findglyph pt_rangew pt_substcode pt_uniname pt_ushow

all: ctests
run: crun prun

ptests:
ctests: $(CTESTS)
prun: ptests $(patsubst %,%.out,$(PTESTS))
crun: ctests $(patsubst %,%.out,$(CTESTS))

clean:
	rm -f *.o *.out
	rm -f $(CTESTS)

pt_%.out: pt_%.ps
	gs -I../res -sDEVICE=nullpage -dBATCH $< > $@
	@grep -q FAIL $@ && (cat $@; rm $@) || true

ct_%.out: ct_%
	./$< > $@
	@grep -q FAIL $@ && (cat $@; rm $@) || true

ct_%.o: ct_%.c
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

ct_%: ct_%.o
	$(CC) -o $@ $(filter %.o,$^) $(LDFLAGS)

ct_deutf: ct_deutf.o u2ps_unicode.o

ct_takecsi: ct_takecsi.o globals.o u2ps_term.o u2ps_tset.o u2ps_tcsi.o u2ps_pswr.o u2ps_unicode.o

ct_strecat: ct_strecat.o psfrem_util.o

ct_u2opts: ct_u2opts.o u2ps_opts.o u2ps_data.o warn.o

ct_fonts1: ct_fonts1.o u2ps_opts.o u2ps_data.o globals.o warn.o
ct_fonts2: ct_fonts2.o u2ps_opts.o u2ps_data.o globals.o warn.o
