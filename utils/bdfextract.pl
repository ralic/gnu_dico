#! /usr/bin/perl
#$Id: bdfextract.pl,v 1.1 2001/11/03 22:34:04 gray Exp $
# This program converts character glyphs from the bdf font file into
# xbm files.
# usage: bdfextract.pl [-v] [-x xlist] [-l list] [-o outdir] file.bdf
#     -v          verbose mode
#     -x xlist    set number translation list. xlist is space-separated list 
#                 of number-name pairs. Each pair consists of the number of the 
#                 character to be extracted followed by the name to be given 
#                 to corresponding output file. 
#     -l list     set number list. list is a list of character numbers to
#                 retrieve from the font file. Output file for the character
#                 <num> will be named <num>.xbm
#     -o outdir   output directory name. Default is ./
#

while ($_ = $ARGV[0], /^-/) {
    shift;
    last if /^--$/;
    /^-d(.*)/ && ($debug = $1);
    /^-v/ && $verbose++;
    /^-x(.*)/ && do {
	@array=split(/[ \t\n]+/, substr($_,2));
	if (scalar(@array) % 2 == 0) {
	    for ($i=0; $i<scalar(@array); $i+=2) {
		$xlist{$array[$i]} = $array[$i+1];
	    }
	} else {
	    warn "Odd number of values in the xlat list ($#array)"
	}
    };
    /^-l(.*)/ && do {
	@array=split(/[ \t\n]+/, substr($_,2));
	for ($i=0; $i<scalar(@array); $i++) {
	    $xlist{$array[$i]} = $array[$i];
	}
    };
    /^-o(.*)/ && do {
	$output_dir=$1;
	if ($output_dir !~ /.*\//) {
	    $output_dir=$output_dir."/"
	}
	if ($verbose) {
	    print "output directory: $output_dir\n"
	}
    }
    # other switches
}

if ($#ARGV > 0) {
    print STDERR "$0: Too many arguments, rest of command line starting from `$ARGV[1]' ignored \n";
}

$in_header=1;
    
while (<>) {
    if ($in_header) {
	if (/FONTBOUNDINGBOX[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+/){
	    ($WIDTH,$HEIGHT)=/FONTBOUNDINGBOX[ \t]+([0-9]+)[ \t]+([0-9]+)[ \t]+[0-9]+[ \t]+[0-9]+/;
	} elsif (/ENDPROPERTIES/) {
	    $in_header=0;
	}
    } elsif ($in_char) {
	if (/BITMAP/ && ($name=wanted_char($in_char))) {
	    if ($verbose) {
		print "found $in_char ($name)\n";
	    }
	    $in_bitmap=$in_char;
	    $in_char=0;
	    $filename=$output_dir.$name.".xbm";
	    if (!open(OUTPUT, ">".$filename)) {
		 print STDERR "$0: Can't open $filename ($!)\n";
		 $in_bitmap=0;
		 next;
	    } 
	    select OUTPUT;
	    print "#define ".$name."_width ".$WIDTH."\n";
	    print "#define ".$name."_height ".$HEIGHT."\n";
	    print "static unsigned char ".$name."_bits[] = {\n";
	} elsif (/ENDCHAR/) {
	    $in_char=0;
	    $in_bitmap=0;
	}
    } elsif ($in_bitmap) {
	if (/ENDCHAR/) {
	    print "};";
	    close OUTPUT;
	    select STDOUT;
	    $in_char=0;
	    $in_bitmap=0;
	} elsif (/[0-9a-fA-F]+/) {
	    ($num)=/([0-9a-fA-F]+)/;
	    $s=`echo $num|utils/revbits`;
	    print "0x".substr($s, 2, 2).", 0x".substr($s, 0, 2).",\n" ;
	}
    } else {
	if (/STARTCHAR[ \t]*[0-9a-fA-F]+/) {
	    ($in_char)=/STARTCHAR[ \t]*([0-9a-fA-F]+)/;
#	    print "$in_char\n";
	}
    }
    last if (eof);	
}

sub wanted_char {
    my($char) = @_;
    while (($key,$value) = each %xlist) {
	return $value if ($key eq $char);
    }
    return 0;
}


