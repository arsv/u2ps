#!/bin/sh

die() { echo "$@"; exit 1; }

echo "reduce-lga" | gs -sDEVICE=nullpage -dWRITESYSTEMDICT -dNOPAUSE -dBATCH -dQUIET \
	-I../res px_reduce_lga.ps ../res/reduce.ps > px_reduce_lga.out \
	|| die "FAIL gs pass 1"

gs -sDEVICE=nullpage -dNOPAUSE -dBATCH -dQUIET ../res/Category/Unidata \
	px_reduce_lga.out >&/dev/null \
	|| die "FAIL gs pass 2"

grep "^\s" px_reduce_lga.out | sort > px_reduce_lga.osr
diff -q px_reduce_lga.chk px_reduce_lga.osr || die "FAIL wrong output"

echo "OK"
