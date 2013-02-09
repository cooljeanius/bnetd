#!/usr/bin/perl

# Usage: convert directory
#  directory:    directory contains old account files 
# from -bb2-pre4 few new chars are escaped before account file is saved to disk. If you want to 
# use this patch, and don't want do loose nick with in example '"' and '.' run this script

use strict;

$ARGV[0] eq "" && die "error arguments"; 
my ($sdir) = $ARGV[0];

opendir(DIR, $sdir) or die "error open dir $sdir"; 
my @files = grep { !/^\.\.?$/ && -f "$sdir/$_" } readdir(DIR); 
closedir DIR; 

foreach my $fn(@files) 
{ 
    chomp $fn;
    my $nfn = $fn;
    my $t = unpack("H2","\."); $t = '%' . uc $t; $nfn =~ s/\./$t/g;
    my $t = unpack("H2","\""); $t = '%' . uc $t; $nfn =~ s/\"/$t/g;
    if ($fn ne $nfn) { rename("$sdir/$fn","$sdir/$nfn") or warn "Couldn't rename $fn to $nfn: $!"; }
}
