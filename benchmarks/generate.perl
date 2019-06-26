#!/usr/bin/perl

$finished = 1;

while ($l = <>) {
  chop($l);
  if ($l =~ /FILE:\s+(.*)/i) {
    $file = $1;
    $file =~ s|\.col||og;
    $file =~ s|\.|_|og;
    if (!$finished) {
      finish();
    }
    $finished = 0;
  } elsif ($l =~ /p edge ([0-9]+) ([0-9]+)/) {
    $n = $1;
    print "  const int $file\[\] = {\n";
    print "    $n, // Number of nodes\n";
    print "    ";
    $cl = 0;
    $files[$nf] = $file; $nf++;
    if ($n < 1000) {
      $cll = 7;
    } else {
      $cll = 6;
    }
  } elsif ($l =~ /e ([0-9]+) ([0-9]+)/) {
    print fmt($1) . "," . fmt($2);
    $cl++;
    if ($cl == $cll) {
      print ",\n    "; $cl=0;
    } else {
      print ", ";
    }
  }
}

if (!$finished) {
  finish();
}

$cl=0;
print "  const int* data\[\] = {\n    ";
for ($i=0; $i<$nf; $i++) {
  print "\&" . $files[$i] . "\[0\],";
  $cl++;
  if ($cl == 4) {
    print "\n    "; $cl = 0;
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
  if ($cl == 4) {
    print "\n    "; $cl = 0;
  } else {
    print " ";
  }
}
print "  };\n\n";

sub finish {
  if ($cl > 0) {
    print "\n    ";
  }
  print "-1,-1\n";
  print "  }\n\n";
  $finished = 1;
}

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

