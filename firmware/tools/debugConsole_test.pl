#!/usr/bin/perl

use strict;
use warnings;
use POSIX qw(:signal_h);
use Time::HiRes qw(usleep);

my $MAXCOLS = 132;
my @PORTS   = ("A", "B", "C", "D", "0");

my $loop  = 1;

#-------------------------------------------------------------------------------------------------------------------------------
#                                                   F U N C T I O N S
#-------------------------------------------------------------------------------------------------------------------------------
sub sigHandler {
	#
	# Description:
	#	Generif signal handler
	#
	my $signum = shift;
	print("$signum-signal has been received\n");
	if ($signum eq "INT" or $signum eq "TERM") {
		$loop = 0;
	}
	return;
}

#-------------------------------------------------------------------------------------------------------------------------------
#                                                       M A I N
#-------------------------------------------------------------------------------------------------------------------------------
my $serialPort = $ARGV[0];
my $err        = 0;

# Checking for arguments
if (not defined($serialPort) or $serialPort eq "") {
	print("ERROR! use $0 <serial port>\n");
	$err = 127;

# Checking for arguments
} elsif (not -e $serialPort) {
	print("ERROR! \"$serialPort\" is not a valid serial port\n");
	$err = 129;

# Serial communication setting
} if (system("stty", "-F", "$serialPort", "9600", "cs8", "-cstopb", "-parenb", "-raw") != 0) {
	print("ERROR! \"$serialPort\" serial port configuration failed\n");
	$err = 131;

# Serial port opening...
} elsif (not open(my $spFH, "> $serialPort")) {
	print("ERROR! I cannot open the serial port \"$serialPort\"\n");
	$err = 133;

} else {
	my $pin  = "";
	my $val  = 0;
	my $size = 0;
	my $msg  = "";

	# Signals Handlers installing
	$SIG{INT}  = \&sigHandler;
	$SIG{TERM} = \&sigHandler;

	while ($loop) {
		#
		# Foo pin status
		#
		$pin = $PORTS[int(rand(scalar @PORTS))] . int(rand(7));
		if ($pin =~ /A[0-9]/) {
			$val = int(rand(255));
		} else {
			$val = int(rand(2));
		}
		print($spFH "$pin:$val\n");

		#
		# Foo log message
		#
		$size = int(rand($MAXCOLS));
		$msg = "";
		for (my $t=0; $t<$size; $t++) {
			$msg = $msg . chr(int(rand(90)+33));
		}
		print($spFH "$msg\n");

		usleep(100000);
	}

	close($spFH);
}
