#!/usr/bin/perl

use strict;
use warnings;

my @frameworks = ( 'Ogg', 'Vorbis', 'libpng', 'SDL', 'SDL_image' );
my $target = 'The Ur-Quan Masters';
my $execfile = "$target.app/Contents/MacOS/$target";

foreach my $fw (@frameworks) {
  my $src = "/Library/Frameworks/$fw.framework";
  my $dest = "$target.app/Contents/Frameworks/$fw.framework";
  system("ditto \"$src\" \"$dest\"");
}

foreach my $fw (@frameworks) {
  my $src = "$target.app/Contents/Frameworks/$fw.framework/$fw";
  my $oldfwid = `otool -L '$src' | head -n 2 | tail -n 1`;
  $oldfwid =~ s/^\s+//;
  $oldfwid =~ s/\s.*$//g;
  my $newfwid = $oldfwid;
  $newfwid =~ s/^\/Library/\@executable_path\/../;

  system("install_name_tool -id $newfwid \"$src\"");
  foreach my $fw2 (@frameworks) {
    next if $fw eq $fw2;
    my $src2 = "$target.app/Contents/Frameworks/$fw2.framework/$fw2";
    system("install_name_tool -change $oldfwid $newfwid \"$src2\"");
  }
  system("install_name_tool -change $oldfwid $newfwid \"$execfile\"");
}
