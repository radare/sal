#!/usr/bin/env perl

$testfile=$ARGV[0];
$sal="sal";
$sal="../src/sal" if (`$sal -V` eq "");
if (`$sal -V` eq "") {
	print "Error: SAL not found in PATH.\n";
	exit(1);
}

my %test;
#eval `cat $testfile`;
require $testfile;

printf "(%15s ) %40s",$testfile, $test->{name}."...  ",$testfile;
open FD, ">/tmp/tmp.sal";
print FD $test->{code};
close FD;

#print ("ENV x".$ENV->{DBG}."y");
if ($ENV{DBG} eq "3") {
	$FLAG="-v";
	$ENV{DBG}="2";
}
chomp($result=`$sal ${FLAG} /tmp/tmp.sal`); chomp($result);
chomp($test->{reply}); chomp($test{reply});


print "[$result] vs [".$test->{reply}."]\n" if ($ENV{DBG} eq "2");

if ($result eq $test->{reply}) {
	print "ok\n";
} else {
	print "failed\n";
	#system("sal -v /tmp/tmp.sal ") if ($ENV{DBG} eq "3");
	print "[$result] vs [".$test->{reply}."]\n" if ($ENV{DBG} eq "1");
}
