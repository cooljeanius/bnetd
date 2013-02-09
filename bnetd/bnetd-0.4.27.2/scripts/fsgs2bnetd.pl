#!/usr/bin/perl

#Usage: fsgs2bnetd "source directory" "destination directory"
#source directory:    directory contains fsgs account files 
#destnation directory: directory to save newly created bnetd 
#                      accounts,should exist before run this 
#there is another change made to bnetd account in this script 
#i think fsgs account filename format <username> is better 
#than using bnetd's <userid>, so i kept it in fsgs format. 
#you may change it by edit the script yourself 

use strict;

$ARGV[1] eq "" && die "error arguments"; 
my $s_dir=$ARGV[0];
my $d_dir=$ARGV[1];

opendir(DIR, $s_dir) || die "error open dir $s_dir"; 
my @files = grep { !/\.\.?\z/ && -f "$s_dir/$_" } readdir(DIR);
closedir DIR; 

my $userid=1; 
my %fsgs = (
'name'	 			=> 'BNET\\\acct\\\username',
'password'			=> 'BNET\\\acct\\\passhash1',
'idnum'				=> 'BNET\\\acct\\\userid',
'fsgs\\watch'			=> 'BNET\\\acct\\\watch',
'fsgs\\ignore'			=> 'BNET\\\acct\\\ignore',
'profile\\age'			=> 'profile\\\age',
'profile\\sex'			=> 'profile\\\sex',
'profile\\location'		=> 'profile\\\location',
'profile\\description'		=> 'profile\\\description',
'fsgs\\lastip'			=> 'BNET\\\acct\\\lastlogin_ip',
'fsgs\\created'			=> 'BNET\\\acct\\\firstlogin_time',
'record\\sexp\\0\\wins'		=> 'Record\\\SEXP\\\0\\\wins',
'record\\sexp\\0\\losses'	=> 'Record\\\SEXP\\\0\\\losses',
'record\\sexp\\0\\disconnects'	=> 'Record\\\SEXP\\\0\\\disconnects',
'record\\sexp\\0\\last game result' => 'Record\\\SEXP\\\0\\\last game result',
'record\\star\\0\\wins'		=> 'Record\\\Star\\\0\\\wins',
'record\\star\\0\\losses'	=> 'Record\\\Star\\\0\\\losses',
'record\\star\\0\\disconnects'	=> 'Record\\\Star\\\0\\\disconnects',
'record\\star\\0\\last game result' => 'Record\\\Star\\\0\\\last game result'
);

foreach (@files) { 
        open(S_FILE, "$s_dir/$_") || die "error open s_file $_"; 
        my $dest_file = "$d_dir/" . lc $_; 
        open(D_FILE, ">$dest_file") || die "error open d_file $_"; 
        while (<S_FILE>) { 
                chop; 
                my ($name, $value) = split(':', $_, 2); 
		next unless exists $fsgs{$name};
		if ( $name eq "password" ) { 
                	$value = passconv($value); 
		} elsif ( $name eq "fsgs\\watch" || $name eq "fsgs\\ignore" ) {
			$value =~ s/\|/,/g;
			chop $value;
		} elsif ( $name eq "record\\sexp\\0\\last game result" || $name eq "record\\star\\0\\last game result" ) {
			$value =~ s/N\.A\./NONE/;
		}
                print D_FILE "\"$fsgs{$name}\"=\"$value\"\n"; 
        } 
        print D_FILE "\"$fsgs{'idnum'}\"=\"$userid\"\n"; 
        $userid++; 
        close(S_FILE); 
        close(D_FILE); 
} 

sub passconv { 
	my $f_pass = $_[0]; 
	my $d_pass = ""; 
	$f_pass = lc $f_pass; 
	my $length = length $f_pass; 
	for (my $i=0; $i <= $length; $i = $i+2) { 
        	$a = 2*(int(($i/2)/4)*8+3-$i/2); 
        	$d_pass .= substr($f_pass, $a, 2); 
	} 
	return $d_pass;
}
