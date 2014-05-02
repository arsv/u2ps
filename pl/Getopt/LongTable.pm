package Getopt::LongTable;

use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(getopt_long_tbl);
@EXPORT_OK = qw(getopt_help);

sub getopt_long_tbl
{
	my $opts = \@_;
	my $arg;
	my $val;

	getopt_default($opts);
	while(@ARGV) {
		$arg = shift @ARGV;
		if($arg eq '--help' || $arg eq '-h') {
			getopt_help($opts);
		} elsif($arg =~ /^--([^=]+)=(.*)/) {
			getopt_long($opts, $1, $2);
		} elsif($arg =~ /^--([^=]+)$/) {
			getopt_long($opts, $1);
		} elsif($arg eq '--') {
			last;
		} elsif($arg =~ /^-(.+)/) {
			getopt_short($opts, $1);
		} else {
			unshift(@ARGV, $arg);
			last;
		}
	};
}

sub getopt_help
{
	my $opts = shift;

	if(defined($opts)) {
		foreach my $o (@$opts) {
			if(!ref($o)) {
				print "$o\n";
			} else {
				getopt_help_option(@$o);
			}
		}
	}
	exit;
}

sub getopt_help_option
{
	my ($short, $long, $arg, $ref, $descr, $default) = @_;

	$opt = '';
	$opt .= "-$short" if defined $short;
	$opt .= "/" if defined $short and defined $long;
	$opt .= "--$long" if defined $long;
	if(ref($ref) && ref($ref) eq 'HASH') {
		$opt .= (defined($long) ? " " : "").$arg;
	} else {
		$opt .= (defined($long) ? "=" : " ").$arg if defined $arg;
	}
	@descr = split(/\n/, $descr);

	printf "    %-20s  %s%s\n", $opt, (shift @descr),
		(defined($default) ? " [$default]" : "");
	foreach my $l (@descr) {
		printf "%26s%s\n", "", $l;
	}
}

sub getopt_default
{
	my $opts = shift;
	foreach my $o (@$opts) {
		${$o->[3]} = $o->[5] if defined($o->[3]) && defined($o->[5]);
	}
}

sub getopt_long
{
	my $opts = shift;
	my $opt = shift;
	my $val = shift;
	my $def;

	foreach my $o (@$opts) {
		next unless ref($o);
		if(defined($o->[1]) && $o->[1] eq $opt) { $def = $o; last; }
	}

	die "Undefined option --$opt\n" unless defined $def;
	die "Option --$opt can't take argument\n" if !defined($def->[2]) && defined($val);
	if(defined($def->[2]) && !defined($val)) {
		$val = shift @ARGV;
		die "Argument required for --$opt\n" unless defined $val;
	}

	${$def->[3]} = (defined($def->[2])) ? $val : 1;
}

sub getopt_short
{
	my $opts = shift;
	my $optc = shift;
	my $opt;
	my $val;
	my $def;
	
	while(length($optc) > 0) {
		$opt = substr($optc, 0, 1);
		$def = undef;
		foreach my $o (@$opts) {
			next unless ref($o); if(defined($o->[0]) && $o->[0] eq $opt) { $def = $o; last; }
		}
		die "Undefined option -$opt\n" unless defined $def;

		if(defined($def->[2])) {
			if(length($optc) > 1) {
				$val = substr($optc, 1);
			} else {
				$val = shift @ARGV;
				die "Argument required for -$opt\n" unless defined $val;
			}
			getopt_apply($def->[3], $val);
			last;
		} else {
			getopt_apply($def->[3]);
			$optc = substr($optc, 1);
		}
	}
}

sub getopt_apply
{
	my $act = shift;
	my $val = shift;

	if(ref($act) eq 'SCALAR') {
		$val = 1 unless defined $val;
		$$act = $val;
	} elsif(ref($act) eq 'CODE') {
		$act->($val);
	} elsif(ref($act) eq 'ARRAY') {
		push(@$act, $val);
	} elsif(ref($act) eq 'HASH') {
		if($val =~ /([^=]+)=(.*)/) {
			$act->{$1} = $2;
		} else {
			$act->{$val} = undef;
		}
	}
}

1;
