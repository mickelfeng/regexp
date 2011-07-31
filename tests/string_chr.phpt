--TEST--
Test utf8_chr function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu

var_dump(utf8_chr());
echo "\n";
var_dump(utf8_chr(63, 64));
echo "\n";
var_dump(utf8_chr(0x1D63C) === $A);
?>
--EXPECTF--
Warning: utf8_chr() expects exactly 1 parameter, 0 given in %s on line %d
NULL


Warning: utf8_chr() expects exactly 1 parameter, 2 given in %s on line %d
NULL

bool(true)
