#!/usr/bin/perl

use strict;
use warnings;

use Cwd 'abs_path';

sub genFirmware {
	my $project = shift(@_);
	my $n = shift(@_);
	chdir $project; 
	system "make clean";
	rename "makefile","makefile.bk";
	my $start_addr = 3;
	my $end_addr = $start_addr + $n;
	for (my $addr = $start_addr; $addr < $end_addr; $addr++) {
		open my $in,'<',"makefile.bk" or die "";
		open my $out,'>',"makefile" or die "";
		while (<$in>) 
		{
			if ($. == 2) 
			{
				print $out "ADDRESS = $addr\n";
				next;
			}
			print $out $_;
		}
		close $in;
		close $out;
		
		system("make");
		system("make clean");
	}
	rename "makefile.bk","makefile";
	chdir "..";
}

my $count = 5;

genFirmware("m8", $count);
genFirmware("m8v", $count);
genFirmware("m16", $count);
genFirmware("m16v", $count);

chdir "pc-link-virtual"; 
system "make clean";
system "make";
chdir "..";

chdir "pc-link"; 
system "make clean";
system "make";
chdir "..";

system "perl gen-map.pl";
system "perl gen-map-pc-link.pl";
system "perl gen-map-pc-link-virtual.pl";

exit 0;