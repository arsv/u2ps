# Options parsing and paper setup.

use strict;
use utf8;

our $GS;
our $BASE;
our $verbose;

our %opts;
our %head;
our %page;
our %term;

our @inc;
our @def;

sub handle_options
{
	parse_options();
	return unless @ARGV;
	
	$opts{wrap} = 1 if defined $opts{wrapmark};

	die "Primary fontset $page{fset} not found\n" unless have_fontset($page{fset});
	die "Auxilliary fontset $page{afset} not found\n" unless have_fontset($page{afset});

	$opts{tenc} = resolve_alias($opts{tenc}) || die "Unknown text encoding $opts{tenc}\n";
	$opts{oenc} = resolve_alias($opts{oenc}) || die "Unknown opts encoding $opts{oenc}\n";
	warn "Using $opts{tenc} for input, $opts{oenc} for options\n" if $verbose;

	my ($pw, $ph, $mm) = papersize($opts{paper});
	my ($mt, $mr, $mb, $ml) = makemargins($opts{margins} || $mm);

	# Save initial paper size as boundingbox
	$page{bw} = $pw;
	$page{bh} = $ph;

	# 90 degress ccw rotation
	($pw, $ph, $ml, $mr, $mt, $mb) = ($ph, $pw, $mb, $ml, $mt, $mr) if $opts{lscape};

	# Effective paper size here
	$page{pw} = $pw;
	$page{ph} = $ph;
	$page{ml} = $ml; $page{mr} = $mr;
	$page{mt} = $mt; $page{mb} = $mb;

	my ($fsize, $cols, $lines) = termsize(
		$pw - $ml - $mr,
		$ph - $mt - $mb,
		$page{fsize}, $term{cols}, $term{lines}
	);
	
	$page{fsize} = $fsize;
	$page{asize} = 0.8*$fsize unless $page{asize};
	$page{lsize} = 0.8*$fsize unless $page{lsize};
	$term{lines} = $lines;
	$term{cols} = $cols;

	foreach my $key (qw(hl hc hr fl fc fr)) {
		(delete $head{$key}, next) unless defined $head{$key};
		$head{$key} = decode($opts{oenc}, $head{$key}, Encode::FB_DEFAULT);
	}

	# A slight look-ahead for input file name here
	$page{title} = decode($opts{oenc}, $ARGV[0], Encode::FB_DEFAULT)
		if !defined($page{title}) && defined($ARGV[0]) && $ARGV[0] ne '-' && $ARGV[0] ne '';

	if(!$opts{noh} && !(keys %head)) {
		$head{hl} = $page{title} if !defined($head{hl}) && defined($page{title});
		$head{hr} = '#' unless defined $head{hr};
	}
}

# A quick check to avoid nasty /undefinedresource errors in gs.
# Unlike fonts, there's no substitution mechanism for fontsets.
sub have_fontset
{
	my $fset = shift;
	return (-f "$BASE/ps/FontSet/$fset") ? 1 : 0;
}

# "A4" -> (W, H, margins)
# margins handled the same way as o_margins, i.e. M, or L:R, or L:R:T:B
sub papersize
{
	my $paper = shift;
	my $pdsc = "$BASE/paper";

	return @page{qw(pw ph)} unless defined $paper;
	return ($1, $2) if $paper =~ /^(\d+)x(\d+)$/;

	open(PAPER, "<", $pdsc) or die "Can't open $pdsc: $!\n";
	while(<PAPER>) {
		next unless /\S/;
		next if /^#/;
		my @data = split(/\s+/);
		next unless @data >= 3 and lc($data[0]) eq lc($paper);
		return @data[1..3];
	}
	close(PAPER);

	die "Unknown paper type \"$paper\"\n";
}

# Turn margin spec into TRBL 4-array
sub makemargins
{
	my $str = shift;
	return @page{qw(mt mr mb ml)} unless defined $str;
	my @d = split(/:/, $str);
	return @d if @d == 4;
	return @d[0,1,0,1] if @d == 2;
	return @d[0,0,0,0] if @d == 1;
	die "Bad margins specification \"$str\"\n";
}

# Given (tw x th) pt terminal area one of (fsize, lines, cols) set,
# figure out the values for the other two.
sub termsize
{
	my ($tw, $th, $fsize, $cols, $lines) = @_;

	# Last-chance fallback. Do everything in 12pt font unless told otherwise.
	$fsize = 12 unless $fsize or $cols or $lines;

	my $fsc = $cols  ? 0.1*(int(20*$tw/$cols)) : 0;
	my $fsl = $lines ? 0.1*(int(10*$th/$lines)) : 0;

	# Bad situation, both lines and cols were specified.
	# Use the largest of two font sizes in this case, adjusting (cutting)
	# the other dimension to make it fit the page.
	($fsc < $fsl) ? undef($fsl) : undef($fsc) if $fsc && $fsl;

	$fsize = $fsc || $fsl || $fsize;

	$cols = int(2*$tw/$fsize);
	$lines = int($th/$fsize);

	warn "Terminal area: ${tw}x${th}pt, font size ${fsize}pt, $cols cols $lines lines\n" if $verbose;

	return ($fsize, $cols, $lines);
}

sub parse_options { getopt_long_tbl(
"Unicode text to PostScript converter",
"Usage:\n",
"    u2ps [options] input-file [output-file]",
"\nInput-related options:",
[ 'e',  'encoding', 'name',	\$opts{tenc},	'Input file encoding', $opts{tenc} ],
[ undef,'opt-encoding', 'name',	\$opts{oenc},	'Options encoding (for --header/--footer)', $opts{oenc} ],
[ 'T',  'tab', 'n',		\$opts{tabstop},'Tab size', $opts{tabstop} ],
[ 'a',  'no-ansi', undef,	\$opts{noansi},	"Do not interpret terminal control sequences" ],
"\nPaper setup:",
[ 'P',  'paper', 'name',	\$opts{paper},	"Paper name, or size in mm (WxH)" ],
[ 'M',  'margins', 'values',	\$opts{margins},"Margins; M, or V:H, or T:R:B:L" ],
[ 'r',  'landscape', undef,	\$opts{lscape},	"Rotate to landscape orientation" ],
"\nFont size selection (use only one of these):",
[ 's',  'font-size', 'n',	\$page{fsize},	"Font size (pt)" ],
[ 'C',  'columns', 'n',		\$term{cols},	"Columns per line" ],
[ 'L',  'lines', 'n',		\$term{lines},	"Lines per page" ],
"\nFonts setup:",
[ 'f',  'font-set', 'name',	\$page{fset},	'Primary font set', 'FreeMono' ],
[ undef,'aux-font-set', 'name',	\$page{afset},	'Auxilliary font set', 'Times-Roman' ],
[ undef,'lnum-font', 'name',	\$page{lfont},	'Font to use for line numbers', 'Times-Roman' ],
[ undef,'head-font-size', 'n',	\$page{asize},	'Headings font size (pt)' ],
[ undef,'lnum-font-size', 'n',	\$page{lsize},	'Line numbers font size (pt)' ],
"\nOutput control options:",
[ 'w',  'wrap', undef,		\$opts{wrap},	'Wrap long lines' ],
[ 'W',  'wrap-mark', undef,	\$opts{wrapmark},'Wrap long lines and set wrap marks' ],
[ 'i',  'inverse', undef,	\$opts{inverse},"White letters on black background" ],
[ 'l',  'numbers', undef,	\$opts{lnum},	'Print line numbers' ],
[ undef,'start-line', 'n',	\$opts{sline},  'First line number', $opts{sline} ],
[ 'b',  'bookish', undef,	\$opts{bookish},'Swap left and right headings on even pages' ],
[ 't',  'title', 'string',	\$page{title},	"Page title; implies -h" ],
[ 'H',  'headings', undef,	\$opts{noh},'Do not print default headings' ],
[ undef,'header-left', '...',	\$head{hl},	'Heading, left aligned' ],
[ undef,'header', '...',	\$head{hc},	'    same, centered' ],
[ undef,'header-right', '...',	\$head{hr},	'    same, right aligned' ],
[ undef,'footer-left', '...',	\$head{fl},	'Footer, left aligned' ],
[ undef,'footer', '...',	\$head{fc},	'    same, centered' ],
[ undef,'footer-right', '...',	\$head{fr},	'    same, right aligned' ],
"\nPostscript options:",
[ 'F',  'no-fonts', undef,	\$opts{nfonts},	"Do not embed fonts" ],
[ 'A', "all-fonts", undef,	\$opts{afonts},	"Embed all fonts, including standard postscript 35" ],
[ undef,'no-notdef', undef,	\$opts{nonotdef},'Do not automatically embed notdef font' ],
[ 'E',  'no-resources',	undef,	\$opts{nopp},	"Do not embed resources / do not post-process output file".
						"(see man page on how to use this)" ],
[ 'd',  'define', 'key',	\@def,		"Passed to gs" ],
[ 'I',  'include', 'path',	\@inc,		"Passed to gs" ],
"\nMisc options:",
[ undef,'keep',	undef,		\$opts{keep},	'Do not delete temporary files' ],
[ 'v',  'verbose', undef,	\$verbose,	'Be verbose' ],
[ 'G',  'gs', 'path',		\$GS,		"Ghostscript command [$GS]" ]
) };

1;
