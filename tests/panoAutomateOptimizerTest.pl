#!/usr/bin/perl

use strict;
use Getopt::Long;

my $createReference = 0;
my $verbose = 0;
GetOptions( '-c'     => \$createReference,
            '--verbose' => \$verbose,
    );

die "Too fee parameters" unless scalar(@ARGV) == 1;

my $input = shift @ARGV;

# the rest of @ARGV is a list of files to use to create the PDD
my $optimizer = '../../tools/PToptimizer';

unlink("tests/$input");
system ($optimizer, '-o', "tests/$input" , "$input") == 0
    or die "executing of $optimizer failed: $?";

if ($createReference) {
    `mv -f tests/$input reference/$input`;
    exit 0;
} else {
    mkdir('tests');
    my $diff = `diff -w 'reference/$input' 'tests/$input' 2>&1`;
    print $diff;
    exit ($diff ne "");
}
