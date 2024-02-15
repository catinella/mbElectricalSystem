#!/usr/bin/perl
#------------------------------------------------------------------------------------------------------------------------------
#
#   __  __       _             _     _ _          _____ _           _        _           _   ____            _                 
# |  \/  | ___ | |_ ___  _ __| |__ (_) | _____  | ____| | ___  ___| |_ _ __(_) ___ __ _| | / ___| _   _ ___| |_ ___ _ __ ___  
# | |\/| |/ _ \| __/ _ \| '__| '_ \| | |/ / _ \ |  _| | |/ _ \/ __| __| '__| |/ __/ _` | | \___ \| | | / __| __/ _ \ '_ ` _ \ 
# | |  | | (_) | || (_) | |  | |_) | |   <  __/ | |___| |  __/ (__| |_| |  | | (_| (_| | |  ___) | |_| \__ \ ||  __/ | | | | |
# |_|  |_|\___/ \__\___/|_|  |_.__/|_|_|\_\___| |_____|_|\___|\___|\__|_|  |_|\___\__,_|_| |____/ \__, |___/\__\___|_| |_| |_|
#                                                                                                 |___/                       
#
# File: debugConsole.pl
#
# Author: Silvano Catinella <catinella@yahoo.com>
#
# Description:
#	This script allows you to display the console messages and the pin status too
#
# License:
#	Copyright (C) 2023 Silvano Catinella <catinella@yahoo.com>
#
#	This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
#	License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
#	version.
#
#	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
#	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License along with this program. If not, see
#		<https://www.gnu.org/licenses/gpl-3.0.txt>.
#
#------------------------------------------------------------------------------------------------------------------------------
use strict;
use warnings;
use Term::Size;
use Time::HiRes qw(gettimeofday usleep);
use Term::ANSIScreen qw(cls);
use POSIX qw(:signal_h);

my $loop  = 1;
my $DEBUG = 1;
my $RATIO = 0.5;
my $tZero = gettimeofday();

sub sigHandler {
	#
	# Description:
	#	Generif signal handler
	#
	my $signum = shift;
#	print("$signum-signal has been received\n");
	if ($signum eq "INT" or $signum eq "TERM") {
		$loop = 0;
	}
	return;
}

sub fill_string {
	#
	# Description:
	#	This function returns a string with the same characters of the argument defined one, but with blank characters
	#	added to achieve the specified size
	#
	# Returned value:
	#	The formatted string
	#
	my $sentence = shift;
	my $size     = shift;
	my $t;
	my $toFill   = ($size - length($sentence));
	my $result   = $sentence;
	
	for ($t = 0; $t < $toFill; $t++) {$result = $result . " "};
	
	return($result);
}

sub CJ_string {
	my $sentence = shift;
	my $maxSize  = shift;
	my $toFill   = ($maxSize - length($sentence)) / 2;
	my $result   = "";

	for (my $t = 0; $t < $toFill; $t++) {$result = $result . " "};
	
	return($result . $sentence);
}

sub line_string {
	my $size = shift;
	my $t;
	my $result = "";

	for ($t = 0; $t < $size; $t++) {$result = $result . "-"};

	return($result);
}

sub formatLog {
	#
	# Description:
	#	This function returns a reduced argument defined string if the size is equal or bigger then the specified max value
	#
	# Returned value:
	#	The formatted string
	#
	my $sentence = shift;
	my $maxSize  = shift;
	my $result   = "";
	my $tStamp   = (gettimeofday() - $tZero);
	my $mxs      = 0;

	chomp($sentence);
	if    ($tStamp <      10) {$result = "00000". $tStamp . ": ";}
	elsif ($tStamp <     100) {$result = "0000" . $tStamp . ": ";}
	elsif ($tStamp <    1000) {$result = "000"  . $tStamp . ": ";}
	elsif ($tStamp <   10000) {$result = "00"   . $tStamp . ": ";}
	elsif ($tStamp <  100000) {$result = "0"    . $tStamp . ": ";}
	else                      {$result =          $tStamp . ": ";}

	$mxs = $maxSize - length($result);

	if (length($sentence) >= $mxs) {
		$result = $result . substr("$sentence", 0, ($mxs - 3)) . "...";

	} else {
		$result = $result . $sentence;
	}

	return($result);
}
#------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------
my $serialPort    = $ARGV[0];
my ($cols, $rows) = Term::Size::chars *STDOUT{IO};
my %signalsDB     = ();
my @msgsList      = ();
my $msgsListIndex = 0;
my $msgsListMaxSz = ((1 - $RATIO) * $rows) - 3;
my $err           = 0;


#foreach (keys(%SIG)) {print("SIG($_): ".$SIG{$_}."\n")}; exit;

# Checking for arguments
if (not defined($serialPort) or $serialPort eq "") {
	print("ERROR! use $0 <serial port>\n");
	$err = 127;

# Checking for arguments
} elsif (not -e $serialPort) {
	print("ERROR! \"$serialPort\" is not a valid serial port\n");
	$err = 129;

# Terminal size retriving
} elsif ($cols == 0 or $rows == 0) {
	print("ERROR! I cannot retrive the current terminal sized\n");
	$err = 131;

# Serial port opening...
} elsif (not open(COM, $serialPort)) {
	print("ERROR! I cannot read the serial port \"$serialPort\"\n");
	$err = 133;

} else {
	my $id;
	my $val;
	my $x     = 0;
	my $y     = 0;
	my $tLine = "";

	# Signals Handlers installing
	$SIG{INT}  = \&sigHandler;
	$SIG{TERM} = \&sigHandler;
	
	while ($loop == 1) {
		#
		# Reading data by the serial port
		#
		$_ = <COM>;
		if (defined($_)) {
			cls();
			chomp;
			if (/^([A-Z0-9][0-9]) *: *([0-9]+)$/) {
				# New pin-setting
				$id  = $1;
				$val = $2;

				$signalsDB{$id} = $2 
	
			} else {
				# New console message
				if ($msgsListIndex >= $msgsListMaxSz) {
					shift(@msgsList);
				} else {
					$msgsListIndex++;
				}
				push(@msgsList, formatLog("$_", $cols));
			}
	

			#
			# PINs list printing
			#
			printf("%s\n", line_string($cols));
			printf("%s\n", CJ_string("P I N s   S t a t u s", $cols));
			printf("%s\n", line_string($cols));
		
			$x = 0;
			$y = 0;
			foreach (keys(%signalsDB)) {
				$tLine = $tLine . fill_string(($_ . ":" . $signalsDB{$_}), int($cols/4));
				$x++;
				if ($x == 4) {
					print("$tLine\n");
					$tLine = "";
					$x = 0;
					$y++;
				}
			}

		
			#
			# Console messages printing
			#
			printf("%s\n", line_string($cols));
			printf("%s\n", CJ_string("C O N S O L E   M E S S A G E S", $cols));
			printf("%s\n", line_string($cols));
		
			foreach(@msgsList) {print("$_\n")};

			usleep(1000);
		}
	}

	print("$$-process is shutting down...\n");
	close(COM);
}

print("\n");
exit($err);
