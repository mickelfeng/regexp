<?php
$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";
$E="\xF0\x9D\x99\x80";
$N1="\xF0\x9D\x9F\x8F"; # 1D7CE, Nd
$N2="\xF0\x9D\x9F\x90";
$N3="\xF0\x9D\x9F\x91";
$N4="\xF0\x9D\x9F\x92";
$N5="\xF0\x9D\x9F\x93";

$r = new Regexp('(\p{L})(\p{Nd})', 'is');

//        0 1  2 3  4 5
$input = "$A$N1$B$N2$C$N3";
//        -6-5 -4-3 -2-1

// =====

echo 'Positive index :', PHP_EOL;

$r->match($input, $m, 1);
echo $m[0] == "$B$N2" ? 'OK' : 'KO', PHP_EOL;

$r->match($input, $m, 3);
echo $m[0] == "$C$N3" ? 'OK' : 'KO', PHP_EOL;

$ret = $r->match($input, $m, 5);
echo $ret === FALSE ? 'OK' : 'KO', PHP_EOL;

$ret = $r->match($input, $m, 10);
echo $ret === FALSE ? 'OK' : 'KO', PHP_EOL;

// =====

echo 'Negative index :', PHP_EOL;

$ret = $r->match($input, $m, -1);
echo $ret === FALSE ? 'OK' : 'KO', PHP_EOL;

$r->match($input, $m, -3);
echo $m[0] == "$C$N3" ? 'OK' : 'KO', PHP_EOL;

$r->match($input, $m, -4);
echo $m[0] == "$B$N2" ? 'OK' : 'KO', PHP_EOL;

$ret = $r->match($input, $m, -6);
echo $m[0] == "$A$N1" ? 'OK' : 'KO', PHP_EOL;

$ret = $r->match($input, $m, -7);
echo $ret === FALSE ? 'OK' : 'KO', PHP_EOL;

// =====

var_dump(
    $r->matchAll($input, $matches),
    $matches
);

// =====

var_dump(
    $r->replace($input, '$2$1')
);

// =====

$r = new Regexp('\d');
var_dump(
    $r->split($input, 2)
);
$r = new Regexp('.');
var_dump(
    $r->split($input)
);