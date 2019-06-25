#!/usr/bin/perl

while ($l = <>) {
  chop($l);
  if ($l =~ /FILE:\s+(.*)/) {
    $file = $1;
    $file =~ s|\.col||og;
  } elsif ($l =~ /p edge ([0-9]+) ([0-9]+)/) {
    $n = $1; $e = $2; $m = 1;
    print "  const int $file\[\] = {\n";
    print "    $n, $e, // Number of nodes and edges\n";
    print "    ";
    $cl = 1;
    $files[$nf] = $file; $nf++;
  } elsif ($l =~ /e ([0-9]+) ([0-9]+)/) {
    print fmt($1) . "," . fmt($2);
    $m++; $cl++;
    if ($m == $e) {
      print "\n  };\n\n"
    } else {
      if ($cl == 8) {
	print ",\n    "; $cl=1;
      } else {
	print ", ";
      }
    }
  }
}

$cl=0;
print "  const int* data\[\] = {\n    ";
for ($i=0; $i<$nf; $i++) {
  print "\&" . $files[$i] . "\[0\],";
  $cl++;
  if ($cl == 6) {
    print "\n    ";
  } else {
    print " ";
  }
}
print "  };\n\n";

$cl=0;
print "  const char* name\[\] = {\n    ";
for ($i=0; $i<$nf; $i++) {
  $f = $files[$i];
  $f =~ s|_|-|og;
  print "\"" . $f . "\",";
  $cl++;
  if ($cl == 6) {
    print "\n    ";
  } else {
    print " ";
  }
}
print "  };\n\n";

sub fmt {
  my $k = $_[0];
  my $s;
  if ($n < 1000) {
    if ($k < 10) {
      $s = "  $k";
    } elsif ($k < 100) {
      $s = " $k";
    } else {
      $s = "$k";
    }
  } else {
    if ($k < 10) {
      $s = "   $k";
    } elsif ($k < 100) {
      $s = "  $k";
    } elsif ($k < 1000) {
      $s = " $k";
    } else {
      $s = "$k";
    }
  }
  return $s;
}

