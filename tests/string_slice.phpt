--TEST--
Test utf8_slice function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";
$E="\xF0\x9D\x99\x80";
$F="\xF0\x9D\x99\x81";
$input = "$A$B$C$D$E$F";

var_dump(utf8_slice($input));
echo "\n";
var_dump(utf8_slice($input, 1, 2, 3));
echo "\n";
var_dump(utf8_slice($input, -1) === $F);
var_dump(utf8_slice($input, -2) === "$E$F");
var_dump(utf8_slice($input, -3, 1) === $D);
var_dump(utf8_slice($input, 0, -1) === "$A$B$C$D$E");
var_dump(utf8_slice($input, 2, -1) === "$C$D$E");
var_dump(utf8_slice($input, 4, -4) === '');
var_dump(utf8_slice($input, -3, -1) === "$D$E");
?>
--EXPECTF--

Warning: utf8_slice() expects at least 2 parameters, 1 given in %s on line %d
NULL


Warning: utf8_slice() expects at most 3 parameters, 4 given in %s on line %d
NULL

bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
