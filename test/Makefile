include ../config.mk

all: test_deutf

clean:
	rm -f *.o

test_%.o: test_%.c
	$(CC) $(CFLAGS) -o $@ -c $<

u2ps_%.o: ../u2ps_%.c
	$(CC) $(CFLAGS) -o $@ -c $<

test_%: test_%.o
	$(CC) -o $@ $(filter %.o,$^)


test_deutf: test_deutf.o u2ps_unicode.o
