use utf8;

sub guess_system_encoding
{
	my $enc;

	return guess_lc_enc('LC_ALL')
		|| guess_lc_enc('LC_CTYPE')
		|| guess_lc_enc('LANG');
}

sub guess_lc_enc
{
	my $var = shift;
	my $val = $ENV{$var};
	return unless defined $val;
	$val =~ s/[^.]+\.//;
	$val =~ s/@.*//;
	return lc($val);
}

sub psstring
{
	my $str = shift;
	return 'null' unless defined $str;
	$str =~ s/([()\\])/\\$1/g;
	return "($str)";
}

sub truefalse
{
	return ($_[0] ? "true" : "false");
}

sub resuffix
{
	my $file = shift;
	my $old = shift;
	my $new = shift;
	
	if(length($file) > length($old) && substr($file, -length($old)) eq $old) {
		return substr($file, 0, length($file) - length($old)).$new;
	} else {
		return $file.$new;
	}
}

sub runcheck
{
	warn(join(" ", @_)."\n") if $verbose;

	my $ret = system(@_);
	return if $ret == 0;
	my $cmd = shift;
	my $reason;

	if(WIFEXITED($ret)) {
		$reason = sprintf('exit status %i', WEXITSTATUS($ret));
	} elsif(WIFSIGNALED($ret)) {
		$reason = sprintf('killed by signal %i', WTERMSIG($ret));
	} elsif(WIFSTOPPED($ret)) {
		$reason = 'stopped';
	} else {
		$reason = 'reason unknown';
	}

	if($verbose) {
		warn 'failed: '.join(' ', $cmd, @_)."\n";
		warn "failed: $reason\n";
		exit(-1);
	} else {
		die "$cmd failed, $reason\n";
	}
}

1;
