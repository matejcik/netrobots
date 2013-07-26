#!/usr/bin/perl

use feature say;
use lib './perl';
use Netrobots;

Netrobots::init(undef);
my $dir = 0;
$| = 1;
my ($x, $y, $damage, $speed, $elapsed) = Netrobots::get_all();
Netrobots::drive(int(rand(360)), 100);
while ($damage < 100) {
  ($x, $y, $damage, $speed, $elapsed) = Netrobots::get_all();
  my $nextdir = int(rand(360));
  Netrobots::drive(0,0) if ($speed == 100 and ($x < 50 or $x > 950 or $y < 50 or $y > 950));
  $nextdir = 540 - $nextdir if ($x < 50) and ($nextdir > 90) and ($nextdir < 270);
  $nextdir = 540 - $nextdir if ($x > 950) and (($nextdir < 90) or ($nextdir > 270));
  $nextdir = 360 - $nextdir if ($y < 50) and ($nextdir > 180);
  $nextdir = 360 - $nextdir if ($y > 950) and ($nextdir < 180);
  Netrobots::drive($nextdir, 100) if ($speed < 50);
  $dir += 10;
  $dir %= 360;
  my $dist = Netrobots::scan($dir, 10);
  Netrobots::cannon($dir, $dist) if $dist;
}
exit 0;
