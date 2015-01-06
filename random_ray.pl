#!/usr/bin/perl -w

use strict;

my $spheres = 100;

print <<HD;
1280 800
4 $spheres 2
1.0 1.0 0.0 0.5
0.0 1.0 1.0 0.5
1.0 0.0 1.0 0.5
1.0 1.0 1.0 0.5
HD

foreach(1..$spheres) {
	print int rand(1280);
	print " ";
	print int rand(800);
	print " ";
	print int rand(50);
	print " ";
	print 10 + int rand(40);
	print " ";
	print int rand(4);
	print "\n";
}


print <<HD;
0.0 240.0 -100.0 1.0 1.0 1.0
640.0 240.0 -10000.0 0.6 0.7 1.0
HD
