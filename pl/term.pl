# Input parsing and PS output. u2ps core essentially.

use strict;
use utf8;
use Text::CharWidth qw(mbwidth);
no strict 'refs';

our $BASE;

our %opts;
our %page;
our %head;
our %term;
our $verbose;

# Postscript output line limiting
my $psllen = 0;			# current line width
my $psllimit = 90;		# desired limit

# Terminal state
# There is no routine to reset these values. That's because u2ps never does.
my $page = 0;
my $hardline = 0;		# input file line
my $softline = 0;		# terminal line, start with 1 on each new page, counts wraps
my $softcol = 0;		# terminal column
my $wrapped = 0;		# current hardline has been wrapped
binmode(STDERR, ":utf8");

# Current terminal style; see rxvt.h starting with RS_Bold definition.
#
# Fg and Bg are color indexes, 0=black, 1=red and so on; see rxvt init.C
#  def_colorName[] for the list. Note u2ps follows 256-color scheme.
# Most of actual work happens in PostScript part, so this is only used
# to ensure correct state at the start of each page.
#
# Keys:
#	bf	CSI 1 m, bold
#	hb	CSI 2 m, faint (half bright)
#	it	CSI 3 m, italic
#	ul	CSI 4 m, underline
#	bl	CSI 5 m, blink
#	rb	CSI 6 m, rapidly blinking
#	rv	CSI 7 m, reverse
#	iv	CSI 8 m, invisible
#	sl	CSI 9 m, crossed out
#
#	fg	foreground color
#	bg	background color
#
#                    1  2  3  4  5  6  7  8  9
my @csi_m_attr = qw(bf hb it ul bl rb rv iv sl);
my %dflt = ((map { $_ => 0 } @csi_m_attr), fg => -1, bg => -1);
my %ansi = %dflt;

sub new_file
{
	print "%!PS-Adobe-2.0\n";
	print "%%BoundingBox: 0 0 $page{bw} $page{bh}\n";
	print "%%Orientation: ".($opts{lscape} ? "Landscape" : "Portrait")."\n";
	print "%%Title: $page{title}\n" if defined $page{title};
	print "%%Pages: (atend)\n";
	print "%%Creator: u2ps\n";
	print "%%CreationDate: ".(localtime)."\n";
	print "%%EndComments\n";
	print "\n";

	print "%%BeginProlog\n";
	print "%%IncludeResource: font notdef\n" unless $opts{nonotdef};
	print "%%IncludeResource: procset fontset\n";
	print "%%IncludeResource: procset uniterm\n";
	print "%%IncludeResource: fontset $page{fset}\n";
	print "%%IncludeResource: fontset $page{afset}\n" if $page{afset} && $page{afset} ne $page{fset};
	print "%%IncludeResource: fontset Times-Roman\n";
	print "%%EndProlog\n";

	readsetup(
		wrapmark => truefalse($opts{wrapmark}),
		bookish => truefalse($opts{bookish}),
		headers => !!(keys %head),
		lnum => $opts{lnum},
		wrap => $opts{wrap},

		ts => $term{tabstop},
		fset => $page{fset},
		hfset => $page{afset},
		lfont => $page{lfont},
		fsize => $page{fsize},
		asize => $page{asize},
		lsize => $page{lsize},

		fg => $opts{inverse} ? '16#FFFFFF' : '16#000000',
		bg => $opts{inverse} ? '16#000000' : '16#FFFFFF',
		hb => '16#AAAAAA',

		ml => $page{ml}, mr => $page{mr},
		mt => $page{mt}, mb => $page{mb},
		pw => $page{pw}, ph => $page{ph},
		bw => $page{bw}, bh => $page{bh}
	);
}

sub readsetup
{
	my $sfile = "$BASE/ps/setup.ps";
	my %opts = @_;
	local $_;

	print "%%BeginSetup\n";
	open(SETUP, '<', $sfile) || die "Can't open $sfile: $!\n";
	while(<SETUP>) {
		next if m!^%#!;
		defined($opts{$2}) ? ($_ = $1.$3) : next if m{^(.*)\s*%\?(\S+)(\s*)$}m;
		s/\$([A-Za-z0-9]+)/defined($opts{$1}) ? $opts{$1} : "null"/ge;
		print;
	};
	close(SETUP);
	print "%%EndSetup\n";
}

sub end_file
{
	end_line();
	end_page();
	print "%%Trailer\n";
	print "%%Pages: $page\n";
	print "%%EOF\n";
}

# This gets called for each input line
sub print_line
{	
	my $func;
	local $_;

	new_line(0);
	foreach(split(/([\n\r\t\x0F\x08\x0C]|\033\[\??[0-9;]*[a-zA-Z]|\033[78=>Fclmno|}~])/, shift)) {
		if(/^\033/ && $opts{noansi}) {
			# suppress output completely; it's very unlikely anyone will ever need
			# raw \033 characters in PS output, but disabling all colors at once
			# may happen quite useful.
			next;
		} elsif(/^\033\[([0-9;]*)([a-zA-Z])/) {
			$func = "csi_$2";
			&$func(split(';', $1)) if defined &$func;
			warn "unknown ANSI seq CSI $1$2\n" if $verbose && !defined &$func;
		} elsif(/^\033\[\?(.*)([a-zA-Z])/) {
			$func = "csi_q_$2";
			&$func(split(';', $1)) if defined &$func;
			warn "unknown ANSI seq CSI ? $1$2\n" if $verbose && !defined &$func;
		} elsif(/^\033(.)/) {
			# ignored
		} elsif($_ eq "\x0C") {
			# form feed
			put_ff();
		} elsif($_ eq "\x0F") {
			# ignored
		} elsif($_ eq "\n") {
			# ignored
		} elsif($_ eq "\r") {
			put_cr();
		} elsif($_ eq "\t") {
			put_tab();
		} elsif($_ eq "\x08") {
			put_bs();
		} elsif($_ ne ""){
			put_text($_);
		}
	}
	return 0;
}

sub csi_m
{
	push(@_, 0) unless @_;
	
	while(@_) {
		my $c = (shift) + 0; # force numeric context
		if($c == 0) {
			ansi_set(%dflt);
		} elsif($c >= 1 && $c <= 9) {
			ansi_set($csi_m_attr[$c-1] => 1);
		} elsif($c >= 21 && $c <= 29) {
			ansi_set($csi_m_attr[$c-20-1] => 0);
		} elsif($c >= 30 && $c < 38) {
			# basic fg color
			ansi_set('fg' => $c - 30);
		} elsif($c == 38) {
			next unless $_[0] == 5; shift; ansi_set('fg' => shift);
		} elsif($c == 39) {
			ansi_set('fg' => -1);
		} elsif($c >= 40 && $c < 48) {
			ansi_set('bg' => $c - 40);
		} elsif($c == 48) {
			next unless $_[0] == 5; shift; ansi_set('bg' => shift);
		} elsif($c == 49) {
			ansi_set('bg' => -1);
		} elsif($c >= 90 && $c <= 99) {
			ansi_set('fg' => $c - 90 + 8);
		} elsif($c >= 100 && $c <= 109) {
			ansi_set('bg' => $c - 100 + 8);
		} else {
			warn "CSI $c: unknown escape sequence\n" if $verbose;
		}
	}
}

sub ansi_set
{
	my %new = @_;
	my %old = %ansi;

	$ansi{$_} = $new{$_} foreach(keys %new);	

	$ansi{efg} = ansi_efg();
	$ansi{ebg} = ansi_ebg();
	$ansi{efn} = ansi_efn();
	
	ps($ansi{efg}.($ansi{efg} =~ /\D/ ? '' : ' fg')) if $ansi{efg} ne $old{efg};
	ps($ansi{ebg}.($ansi{ebg} =~ /\D/ ? '' : ' bg')) if $ansi{ebg} ne $old{ebg};
	ps($ansi{efn}) if $ansi{efn} ne $old{efn};
	ps($new{ul} ? 'ul' : 'ue') if $new{ul} != $old{ul};
	ps($new{sl} ? 'sl' : 'se') if $new{sl} != $old{sl};
}

# The main problem here is to avoid using indexed colors for non-color situations
# like faint, inverse and so on. This way we can omit idexed color definition later.
sub ansi_efg
{
	return ($ansi{fg} + 8) if $ansi{fg} >= 0 && $ansi{fg} <= 7 && $ansi{bf};
	return $ansi{fg} if $ansi{fg} >= 0;
	return "vf" if $ansi{iv};
	return "hf" if $ansi{hb};
	return "rf" if $ansi{rv};
	return "nf";
}

sub ansi_ebg
{
	return $ansi{bg} if $ansi{bg} >= 0;
	return "vg" if $ansi{iv};
	return "rg" if $ansi{rv};
	return "ng";
}

sub ansi_efn
{
	my $ebf = ($ansi{bf} or $ansi{bl} or $ansi{rb});
	return "fO" if $ebf and $ansi{it};
	return "fI" if $ansi{it};
	return "fB" if $ebf;
	return "fR";
}

# Print a chunk of text, possibly starting new line.
# All wrapping occurs here
sub put_text
{
	my $str = shift;
	#warn "wrapping if off\n" unless $o_wrap and $verbose;
	return pt($str) unless $opts{wrap};
	
	warn "put_text with soft-line=$softline\n" if $softline > $term{lines};

	#warn "trying to wrap line (width=$g_cols, col=$c_soft_col, softline=$c_soft_line)\n" if $verbose;
	my @str = split(//, $str);
	my $tmp = '';

	foreach my $c (@str) {
		my $w = mbwidth($c);
		if($softcol > 0 && $softcol + $w > $term{cols}) {
			# line wrap — flush current buffer and start new line
			pt($tmp) if length($tmp) > 0;
			new_line(1);
			$softcol = $w;
			$tmp = $c;
		} else {
			# still fitting on current line
			$tmp .= $c;
			$softcol += $w;
		}
	}
	#warn "\tdone, col=$c_soft_col\n" if $verbose;
	pt($tmp) if length($tmp) > 0;
}

sub new_line
{
	my $wrapped = shift;
	my $nopb = shift;

	end_line($wrapped);
	new_page() if ($softline % $term{lines} == 0) && !$nopb;

	$softline++;
	$softcol = 0;
	$hardline++ unless $wrapped;

	ps("$hardline l") if $opts{lnum} && !$wrapped;
}

sub new_page
{
	end_page();

	my $lref = $hardline + $opts{sline} - 1;
	my %attr = %ansi;
	%ansi = ( );

	$page++;
	$softcol = 0;
	$softline = 0;

	print "%%Page: $page $page\n";
	ps("save") if $opts{inverse};
	ps($page % 2 ? "odd" : "even") if $opts{bookish};
	ps("la") if $opts{lscape};
	ps("bk") if $opts{inverse};
	foreach my $k (qw(hl hc hr fl fc fr)) {
		my $v = $head{$k};
		next unless defined $v;
		$v =~ s/(^|[^\\])#/$1$page/g;
		my $c = $k;
		$c =~ tr/lr/io/ if $opts{bookish};
		$c =~ s/^(.)/\u$1/;
		ps(psstring($v)." ".$c);
	}
	ps("tr");
	ansi_set(%attr);
	ps("\n\n");
}

sub end_line
{
	my $wrapped = shift;
	ps(($wrapped ? "w" : "n")."\n") if $softline;
}

sub end_page
{
	print "\nshowpage" if $softline;
	print " restore" if $softline && $opts{inverse};
	print "\n" if $softline;
	print "\n";
}


sub put_tab
{
	ps("t");
	my $mod = $softcol % $term{tabstop};
	$softcol += ($mod == 0) ? $term{tabstop} : ($term{tabstop} - $mod);
}

sub put_cr
{
	ps("cr\n");
	$softcol = 0;
}

sub put_bs
{
	return if $softcol == 0;
	ps("bs");
	$softcol--;
}

sub put_ff
{
	warn "form feed\n" if $verbose;
	new_page();
	new_line(0, 1);
}

sub usplitwidth
{
	my $max = shift;
	my $str = shift;
	my $len = shift || 0;
	my @str = split(//, $str);
	my @ret = '';
	my $ret;

	foreach my $c (@str) {
		my $w = mbwidth($c);
		if($len > 0 && $len + $w > $max) {
			push(@ret, [ $ret, $len ]);
			$ret = $c;
			$len = $w;
		} else {
			$ret .= $c;
			$len += $w;
		}
	}
	push(@ret, [ $ret, $len ]);

	return @ret;
}

sub pt
{
	my $text = shift;
	local $_;
	while(length($_ = substr($text, 0, $psllimit-4, ''))) {
		s/([()\\])/\\$1/g;
		ps("($_)u");
	}
}

# print raw piece of postscrip code, wrapping *postscript* line if needed
sub ps
{
	my $code = shift;
	my $nl = ($code =~ /(\n+)$/s ? $1 : undef);
	$code =~ s/\n+$//s;

	my $len = length($code);
	my $slen = ($psllen ? 1 : 0) + $len;

	if($psllen + $slen < $psllimit || $psllen < 10) {
		print " " if $psllen;
		$psllen += $slen;
	} else {
		# nope
		print "\n";
		$psllen = $len;
	}
	print $code;
	
	return unless defined $nl;
	print $nl;
	$psllen = 0;
}

1;
