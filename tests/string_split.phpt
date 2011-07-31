--TEST--
Test utf8_split function
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
$input = "$A$B$C$D$E";

var_dump(utf8_split());
echo "\n";
var_dump(utf8_split($input, 0));
echo "\n";
var_dump(utf8_split($input, 1, 2));
echo "\n";
var_dump(utf8_split($input, 1) === array("$A", "$B", "$C", "$D", "$E"));
var_dump(utf8_split($input, 2) === array("$A$B", "$C$D", "$E"));
?>
--EXPECTF--

Warning: utf8_split() expects at least 1 parameter, 0 given in %s on line %d
NULL


Warning: utf8_split(): Length of each segment must be greater than zero in %s on line %d
bool(false)


Warning: utf8_split() expects at most 2 parameters, 3 given in %s on line %d
NULL

bool(true)
bool(true)
