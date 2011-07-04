--TEST--
Test utf8_reverse function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";

var_dump(utf8_reverse());
var_dump(utf8_reverse("hello world !", 2));
var_dump(utf8_reverse(''));
var_dump(utf8_reverse("$A$B$C$D") === "$D$C$B$A" ? 'OK' : 'FAILED');
var_dump(utf8_reverse('élève'));
?>
--EXPECTF--
Warning: utf8_reverse() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_reverse() expects exactly 1 parameter, 2 given in %s on line %d
NULL
string(0) ""
string(2) "OK"
string(%d) "evèlé"
