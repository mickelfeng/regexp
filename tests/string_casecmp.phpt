--TEST--
Test utf8_casecmp function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

var_dump(utf8_casecmp('içmek'));
var_dump(utf8_casecmp('içmek', 'İÇMEK', 3));

var_dump(utf8_casecmp('iki', 'İKİ'));     // locale not considered (for now ?): not equal
var_dump(utf8_casecmp('sıcak', 'SICAK')); // locale not considered (for now ?): not equal

var_dump(utf8_casecmp('élève', 'ÉLÈVE')); // equal
var_dump(utf8_casecmp('ÉLÈVE', 'Élève')); // equal

var_dump(utf8_casecmp("a\xCC\x80bc", "\xC3\x80bc")); // decomposition test, equal
?>
--EXPECTF--

Warning: utf8_casecmp() expects exactly 2 parameters, 1 given in %s on line %d
NULL

Warning: utf8_casecmp() expects exactly 2 parameters, 3 given in %s on line %d
NULL
int(-%d)
int(%d)
int(0)
int(0)
int(0)
