--TEST--
Test utf8_casecmp function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

var_dump(utf8_casecmp('içmek'));
var_dump(utf8_casecmp('içmek', 'İÇMEK', 'tr', 3));

var_dump(utf8_casecmp('iki', 'İKİ'));     // default case folding (not equal)
var_dump(utf8_casecmp('sıcak', 'SICAK')); // default case folding (not equal)

var_dump(utf8_casecmp('iki', 'İKİ', 'tr'));     // locale case folding (equal)
var_dump(utf8_casecmp('sıcak', 'SICAK', 'tr')); // locale case folding (equal)

var_dump(utf8_casecmp('élève', 'ÉLÈVE')); // equal
var_dump(utf8_casecmp('ÉLÈVE', 'Élève')); // equal

//var_dump(utf8_casecmp("a\xCC\x80bc", "\xC3\x80bc")); // decomposition test, equal
?>
--EXPECTF--

Warning: utf8_casecmp() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: utf8_casecmp() expects at most 3 parameters, 4 given in %s on line %d
NULL
int(%i)
int(%i)
int(0)
int(0)
int(0)
int(0)
