#!/usr/bin/perl

use warnings;
use strict;

my $nota = 0;
my $mess = "";

open(INFO,"< infochecker.txt") or die "ERROR opening infochecker.txt";
while (<INFO>) {
	my ($param, $grep, $in, $res, $mark) = split;
	if ( $param eq "execute" ) {
		system "./cl $param < $in > /dev/null";
		system "diff $res outputexecution > /dev/null";
	} else {
		system "./cl $param < $in | grep \"^$grep\" | sort | uniq > hola";
		system "grep \"^$grep\" $res | sort | uniq > hola2";
		system "diff hola hola2 > /dev/null";
	}
	if ( $? ) {
		$mess = "$mess no";
	} else {
		$nota += $mark;
		$mess = "$mess si";
	}
}
close(INFO);

print "$mess $nota\n";

open(NOTA,"> ../nota") or die "ERROR opening ../nota";
print NOTA $nota;
close(NOTA);
open(NOTA,"> nota") or die "ERROR opening nota";
print NOTA "$mess $nota";
close(NOTA);
