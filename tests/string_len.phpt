--TEST--
Test utf8_len function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";
$E="\xF0\x9D\x99\x80";
$F="\xF0\x9D\x99\x81";
$input = "$A$B$C$D$E$F";

var_dump(utf8_len());
echo "\n";
var_dump(utf8_len($input, 0));
echo "\n";
echo utf8_len($input), "\n";
?>
--EXPECTF--

Warning: utf8_len() expects exactly 1 parameter, 0 given in %s on line %d
NULL


Warning: utf8_len() expects exactly 1 parameter, 2 given in %s on line %d
NULL

6
