#! /usr/local/bin/perl -w
use strict;


if($#ARGV == -1) {
  print "Usage:\n\nperl iscas2tb.pl <file_with_test_vectors> <file_with_input_names>\n\n\nInput names should be just copied from Verilog file\n";
exit(0);
}

open(VALFILE, "<", $ARGV[0]) or die "Failed to open file with values!\n";
open(NAMFILE, "<", $ARGV[1]) or die "Failed to open file with names!\n";
my $namesLine = do {local $/; <NAMFILE>};
$namesLine =~ s/\n\r *//g;
my @names = split /,/,$namesLine;
my @values = <VALFILE>;
close(VALFILE);
close(NAMFILE);
my $outfileName = $ARGV[0].".testbench";
open(TESTBENCH, ">", $outfileName);
my $time = 0;
foreach my $valuesLine (@values) {
  
  $valuesLine =~ s/[\n\r *]//g;
  my @values = split(//,$valuesLine);
  
  print TESTBENCH "#$time: ";
  my $ki = 0;
  foreach my $name (@names) {
    
    $name =~ s/[\n\r *]//g;
    print TESTBENCH $name." = ".$values[$ki].";\n";
    
    $ki = $ki + 1;
  }
  $time = $time + 10;
}
close(TESTBENCH);



