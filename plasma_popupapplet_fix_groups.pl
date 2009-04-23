#!/usr/bin/perl

while(<>) {
  if ($_ =~ m/^\[.*\[Configuration\]\[PopupApplet\]$/) {
    print "# DELETEGROUP $_";
    $_ =~ s/\[Configuration\]\[PopupApplet\]$/[PopupApplet]/;
  }
  print $_;
}
