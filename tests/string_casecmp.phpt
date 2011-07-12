--TEST--
Test utf8_casecmp function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

var_dump(utf8_casecmp('Içmek'));
var_dump(utf8_casecmp('Içmek', 'IÇMEK', 3));

var_dump(utf8_casecmp('Içmek', 'İÇMEK')); // locale not considered : not equal
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
int(0)
int(0)
int(0)
