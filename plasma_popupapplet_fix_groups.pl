#!/usr/bin/perl

while(<>) {
  if ($_ =~ m/^\[.*\[Configuration\]\[PopupApplet\]$/) {
    $_ =~ s/\[Configuration\]\[PopupApplet\]$/[PopupApplet]/;
  }
  print $_;
}
