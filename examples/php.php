#!/usr/bin/php5 -d extension_dir=php/
<?php

include "./php/netrobots.php";

$f = fopen("/tmp/xx", "w");

#netrobots_init($argc, $argv);
netrobots::init(1, [ "PHP" ]);

$dir = 0;
$drivedir = rand(0,360);
$all = netrobots::get_all();
var_dump($all);
flush();
list($x, $y, $damage, $speed, $elapsed) = netrobots::get_all();
netrobots::drive($drivedir, 100);
while ($damage < 100) {
  list($x, $y, $damage, $speed, $elapsed) = netrobots::get_all();
  fprintf($f, "%d %d\n", $x, $y);
  if ($speed == 100 && ($x < 50 || $x > 950 || $y < 50 || $y > 950))
    netrobots::drive(0,0);
  
  $nextdir = rand(0,360);
  if (($x < 50) && ($nextdir > 90) && ($nextdir < 270))
    $nextdir = 540 - $nextdir;
  if (($x > 950) && (($nextdir < 90) || ($nextdir > 270)))
    $nextdir = 540 - $nextdir;
  if (($y < 50) && ($nextdir > 180))
    $nextdir = 360 - $nextdir;
  if (($y > 950) && ($nextdir < 180))
    $nextdir = 360 - $nextdir;
  if (($speed < 50)) {
    $drivedir = $nextdir;
    netrobots::drive($drivedir, 100);
  }
  $dir += 10;
  $dir %= 360;
  $dist = netrobots::scan($dir, 10);
  if ($dist)
    netrobots::cannon($dir, $dist);
}

echo "bagr\n";

?>
