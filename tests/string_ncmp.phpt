--TEST--
Test utf8_ncmp function
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

var_dump(utf8_ncmp("abc", "def")); //invalid
var_dump(utf8_ncmp("abc", "def", 0, 'tr', 4)); // invalid
var_dump(utf8_ncmp("abc", "def", -1)); // -1 is invalid
var_dump(utf8_ncmp("abc", "def", 0)); // == expected (0)
var_dump(utf8_ncmp("$A$B$C$D$E", "$A$B$D", 2)); // == expected (0)
var_dump(utf8_ncmp("$A$B$D", "$A$B$C$D$E", 2)); // == expected (0), same but inverted
var_dump(utf8_ncmp("$A$B$C$D$E", "$A$B$C$D$E", 200)); // == expected (0)
var_dump(utf8_ncmp("$A$B$C$D$E", "$B$A$C$D$E", 2)); // < expected (-%d)
var_dump(utf8_ncmp('', "$A$B$C", 100)); // < expected (-%d)
?>
--EXPECTF--

Warning: utf8_ncmp() expects at least 3 parameters, 2 given in %s on line %d
NULL

Warning: utf8_ncmp() expects at most 4 parameters, 5 given in %s on line %d
NULL

Warning: utf8_ncmp(): Length must be greater than or equal to 0 in %s on line %d
bool(false)
int(0)
int(0)
int(0)
int(0)
int(%i)
int(%i)
