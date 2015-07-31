#!/bin/sh

die() { echo "$@"; exit 1; }

echo "reduce-rgl" | gs -sDEVICE=nullpage -dWRITESYSTEMDICT -dNOPAUSE -dBATCH -dQUIET \
	-I../res px_reduce_rgl.ps ../res/reduce.ps > px_reduce_rgl.out \
	|| die "FAIL gs pass 1"

gs -sDEVICE=nullpage -dNOPAUSE -dBATCH -dQUIET px_reduce_rgl.out >&/dev/null \
	|| die "FAIL gs pass 2"

grep "^\s" px_reduce_rgl.out | sort > px_reduce_rgl.osr
diff -q px_reduce_rgl.chk px_reduce_rgl.osr || die "FAIL wrong output"

echo "OK"
