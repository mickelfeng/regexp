--TEST--
Test utf8_ord function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";

var_dump(utf8_ord($A));
echo "\n";
var_dump(utf8_ord($A, 3, NULL));
echo "\n";
echo utf8_ord("$A$B$C", 1), "\n";
?>
--EXPECTF--
Warning: utf8_ord() expects exactly 2 parameters, 1 given in %s on line %d
NULL


Warning: utf8_ord() expects exactly 2 parameters, 3 given in %s on line %d
NULL

120381
