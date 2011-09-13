--TEST--
Test utf8_casecmp function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

var_dump(utf8_casecmp('içmek'));
var_dump(utf8_casecmp('içmek', 'İÇMEK', 'tr', 3));

echo "Special case I with a default case folding:\n";
var_dump(utf8_casecmp('iki', 'İKİ'));     // default case folding (not equal)
var_dump(utf8_casecmp('sıcak', 'SICAK')); // default case folding (not equal)

echo "Special case I with a TR locale:\n";
var_dump(utf8_casecmp('iki', 'İKİ', 'tr'));     // locale case folding (equal)
var_dump(utf8_casecmp('sıcak', 'SICAK', 'tr')); // locale case folding (equal)

echo "Some standard cases (common case folding):\n";
var_dump(utf8_casecmp('élève', 'ÉLÈVE')); // equal
var_dump(utf8_casecmp('ÉLÈVE', 'Élève')); // equal

echo "Full case folding:\n";
var_dump(utf8_casecmp("{$ffi}{$ffi}${ffi}", "F{$fi}${ff}IFFI"));
var_dump(utf8_casecmp("ABC{$N03B0}DEF{$N0587}GHI{$N1FB7}JKL", "abc{$C03B0}def{$C0587}ghi{$C1FB7}jkl"));
?>
--EXPECTF--

Warning: utf8_casecmp() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: utf8_casecmp() expects at most 3 parameters, 4 given in %s on line %d
NULL
Special case I with a default case folding:
int(%i)
int(%i)
Special case I with a TR locale:
int(0)
int(0)
Some standard cases (common case folding):
int(0)
int(0)
Full case folding:
int(0)
int(0)
