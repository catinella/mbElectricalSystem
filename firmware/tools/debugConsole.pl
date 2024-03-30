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
use POSIX qw(:signal_h);
use Term::Size;
use Time::HiRes qw(gettimeofday usleep);
use Term::ANSIScreen qw(cls);
use IO::Select;
use Cwd qw(abs_path);
#use Device::SerialPort;

my $loop  = 1;
my $DEBUG = 1;
my $RATIO = 0.5;
my $tZero = gettimeofday();
my $PINSMAPFILE = "../mbesPinsMap.h";
my $HWCONFFILE = "../mbesHwConfig.h";


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

	$tStamp =~ s/^([0-9]+\.[0-9]{3})[0-9]*/$1/;   # 3 digits after the point

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


sub PinNameDB_fill {
	#
	# Description:
	#	This funtion fills the argument defined pins-names-db using the C-definitions belong to the "../mbesPinsMap.h"
	#	 header file
	#	
	# Returned value:
	#	The number of valid recognized symbol definitoions
	#
	my $dbRef = shift;
	my $counter = 0;
	
	if (open(FH, "<$PINSMAPFILE")) {
		while (<FH>) {
			if (/^[ \t]*#define[ \t]([^\t ]+)[ \t]+"([^ \t]+)"/) {
				$dbRef->{"$2"} = "$1";
				$counter++;
			}
		}
	} else {
		$counter = -1;
	}
	
	return($counter);
}


sub clearScreen {
	#
	# Description:
	#	This function clears the ASCII terminal screen
	#	[!] Consider, tput is fastest then the other methods and clear also the terminal buffer memory
	#
	my $tputCmd = "/usr/bin/tput";
	if (-x $tputCmd) {
		system("$tputCmd reset");
	} else {
		cls();
	}

	return(1);
}


sub getRS232bps {
	#
	# Description:
	#	It reads the RS232-bps setting from the $HWCONFFILE file
	#
	#
	my $outRef = shift;
	my $err    = -1;

	if (open(FH, "< $HWCONFFILE")) {
		while (<FH>) {
			if (/^[ \t]*#define[ \t]+RS232_BPS[ \t]+([^ \t]+)/) {
				$$outRef = $1;
				$err = 1;
			}
		}
		close(FH);
	}

	return($err);
}

#------------------------------------------------------------------------------------------------------------------------------
#                                                       M A I N
#------------------------------------------------------------------------------------------------------------------------------
my $serialPort    = $ARGV[0];
my ($cols, $rows) = Term::Size::chars *STDOUT{IO};
my %signalsDB     = ();
my @msgsList      = ();
my $msgsListIndex = 0;
my $msgsListMaxSz = ((1 - $RATIO) * $rows) - 3;
my $err           = 0;
my %pinsMap       = ();
my $rs232BPS      = 0;


chdir(abs_path($0));

# RS232 BPS setting reading...
if (getRS232bps(\$rs232BPS) < 0) {
	print("ERROR! I cannot find the RS232 settings in the \"$HWCONFFILE\" header file\n");
	$err = 130;

# PINs Symbols data retriving
if (PinNameDB_fill(\%pinsMap) < 0) { 
	print("ERROR! I cannot read the \"$PINSMAPFILE\" header file\n");
	$err = 127;

# Checking for arguments
} elsif (not defined($serialPort) or $serialPort eq "") {
	print("ERROR! use $0 <serial port>\n");
	$err = 128;

# Checking for arguments
} elsif (not -e $serialPort) {
	print("ERROR! \"$serialPort\" is not a valid serial port\n");
	$err = 129;

# Terminal size retriving
} elsif ($cols == 0 or $rows == 0) {
	print("ERROR! I cannot retrive the current terminal sized\n");
	$err = 131;

# Serial communication setting
} if (system("stty", "-F", "$serialPort", "9600", "cs8", "-cstopb", "-parenb", "-raw") != 0) {
	print("ERROR! \"$serialPort\" serial port configuration failed\n");
	$err = 133;

# Serial port opening...
} elsif (not open(my $spFH, "< $serialPort")) {
	print("ERROR! I cannot read the serial port \"$serialPort\"\n");
	$err = 135;

} else {
	my $x     = 0;
	my $y     = 0;
	my $tLine = "";
	my $sel   = IO::Select->new();
	my @ready = ();
	my @arr   = ();
	my $pin   = "";
	

#	# Serial port interface setting
#	{
#		my $sPort = Device::SerialPort->($serialPort);
#		$sPort->baudrate(9600);
#		$sPort->databits(8);
#		$sPort->parity(none);
#		$sPort->stop(1);
#	}

	$sel->add($spFH);

	# Signals Handlers installing
	$SIG{INT}  = \&sigHandler;
	$SIG{TERM} = \&sigHandler;
	
	while ($loop == 1) {
		@ready = $sel->can_read(0.5);
		
		if (scalar @ready > 0) {

			#
			# Reading data by the serial port
			#
			$_ = <$spFH>;
			if (defined($_) and $_ ne "\n") {
				chomp;
				if (/^[^A-Za-z\s0-9]*([A-Z0-9][0-9]) *: *([0-9]+)\s*$/) {
					# New pin-setting
					$signalsDB{$1} = $2 
	
				} else {
					# New console message
					if ($msgsListIndex >= $msgsListMaxSz) {
						shift(@msgsList);
					} else {
						$msgsListIndex++;
					}
					push(@msgsList, formatLog("$_", $cols));
				}
			}
		}

		clearScreen();


		#
		# PINs list printing
		#
		printf("%s\n", line_string($cols));
		printf("%s\n", CJ_string("P I N s   S T A T U S", $cols));
		printf("%s\n", line_string($cols));
		
		$x = 0; $y = 0;
		$tLine = "";
		@arr   = sort keys %signalsDB;
		$pin   = "";
		foreach (@arr) {
			$pin = $pinsMap{"$_"};
			if (not defined $pin) {$pin = $_};
			
			$tLine = $tLine . fill_string(("$pin:" . $signalsDB{$_}), int($cols/4));
			$x++;
			if ($x == 4) {
				print("$tLine\n");
				$tLine = "";
				$x = 0;
				$y++;
			}
		}
		if ($x < 4) {
			print("$tLine\n");
		}

		
		#
		# Console messages printing
		#
		printf("%s\n", line_string($cols));
		printf("%s\n", CJ_string("C O N S O L E   M E S S A G E S", $cols));
		printf("%s\n", line_string($cols));

		foreach(@msgsList) {print("$_\n")};

		# STDOUT flushing..
		$| = 1;
	}

	print("$$-process is shutting down...\n");
	close($spFH);
}

print("\n");
exit($err);
