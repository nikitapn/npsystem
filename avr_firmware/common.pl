# 
sub get_data_seg 
{
	my $linesref = shift;
	my @lines = @{$linesref};
	
	my $symbol = shift;

	foreach my $line (@lines) 
	{
		if ($line=~m/^(\S+)(\s\S\s)($symbol)$/) 
		{
			return (hex $1) & 0xFFFF;
		}
	}
	die "Cannot find $symbol Parse Error";
}

1;