#!/usr/bin/perl

# Usage: convert directory
#  directory:    directory contains old account files 
# 	
# Converts UID format user file to name format. Also changes all chars to lowercase
# for compatibility with dynamic account load.

use strict;

$ARGV[0] eq "" && die "error arguments"; 
my $sdir=$ARGV[0];

opendir(DIR, $sdir) || die "error open dir $sdir"; 
my @files = grep { /^[^.]/ && -f "$sdir/$_" } readdir(DIR); 
closedir DIR; 


foreach my $fn(@files) 
{ 
    chomp($fn);
    if ($fn =~ /\D/) 
    {
	my $nfn = lc $fn;
	unless ($fn eq $nfn) { rename ("$sdir/$fn", "$sdir/$nfn") or die "Couldn't rename $fn: $!"; }
    } 
    else 
    {
	open (S_FILE, "$sdir/$fn") or die "Couldn't open $fn: $!";
	while (my $line = <S_FILE>)
	{
	    chomp($line);
	    my($key,$value) = split ('=',$line,2);
	    if ($key eq "\"BNET\\\\acct\\\\username\"")
	    {
	    	my $nfn = lc $value;
		$nfn =~ s/\"//g;
		my $t = unpack("H2",'%'); $t = '%' . uc $t; $nfn =~ s/%/$t/g;
		my $t = unpack("H2","\/"); $t = '%' . uc $t; $nfn =~ s/\//$t/g;
		my $t = unpack("H2","\\"); $t = '%' . uc $t; $nfn =~ s/\\/$t/g;
		my $t = unpack("H2","\:"); $t = '%' . uc $t; $nfn =~ s/\:/$t/g;
		my $t = unpack("H2","\|"); $t = '%' . uc $t; $nfn =~ s/\|/$t/g;
		my $t = unpack("H2","\*"); $t = '%' . uc $t; $nfn =~ s/\*/$t/g;
		my $t = unpack("H2","\?"); $t = '%' . uc $t; $nfn =~ s/\?/$t/g;
		my $t = unpack("H2","\<"); $t = '%' . uc $t; $nfn =~ s/\</$t/g;
		my $t = unpack("H2","\>"); $t = '%' . uc $t; $nfn =~ s/\>/$t/g;
		
		
		rename("$sdir/$fn","$sdir/$nfn");
		last;
	    }
	}
	close(S_FILE);
    }
}
