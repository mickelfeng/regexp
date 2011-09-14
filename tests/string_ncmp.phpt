--TEST--
Test utf8_ncmp function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

$input = "$A$B$C$D$E";

var_dump(utf8_ncmp("abc", "def")); //invalid
var_dump(utf8_ncmp("abc", "def", 0, 4)); // invalid
var_dump(utf8_ncmp("abc", "def", -1)); // -1 is invalid
var_dump(utf8_ncmp("abc", "def", 0)); // == expected (0)
var_dump(utf8_ncmp("$A$B$C$D$E", "$A$B$D", 2)); // == expected (0)
var_dump(utf8_ncmp("$A$B$D", "$A$B$C$D$E", 2)); // == expected (0), same but inverted
var_dump(utf8_ncmp("$A$B$C$D$E", "$A$B$C$D$E", 200)); // == expected (0)
var_dump(utf8_ncmp("$A$B$C$D$E", "$B$A$C$D$E", 2)); // < expected (-%d)
var_dump(utf8_ncmp('', "$A$B$C", 100)); // < expected (-%d)
?>
--EXPECTF--

Warning: utf8_ncmp() expects exactly 3 parameters, 2 given in %s on line %d
NULL

Warning: utf8_ncmp() expects exactly 3 parameters, 4 given in %s on line %d
NULL

Warning: utf8_ncmp(): Length must be greater than or equal to 0 in %s on line %d
bool(false)
int(0)
int(0)
int(0)
int(0)
int(%i)
int(%i)
