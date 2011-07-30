--TEST--
Test utf8_slice_count function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";

// Invalid prototypes
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C"));
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", $A, 0, 1, FALSE));

var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", $A, 0, 0)); // 0
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", $A, 0, -3));

// Overlap so 1 not 2
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", "$A$B$C$A")); // 1

// Offset test
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", "$B$C", 2)); // 2
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", $B, -5)); // 2
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", $C, -5, 2)); // 1

// Code point conversion
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", 0x1D63E, 1, -1)); // 2

// Not found
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", 0x1D63F, 1, -1)); // 0
var_dump(utf8_slice_count("$A$B$C$A$B$C$A$B$C", $D)); // 0
?>
--EXPECTF--

Warning: utf8_slice_count() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: utf8_slice_count() expects at most 4 parameters, 5 given in %s on line %d
NULL
int(0)
int(2)
int(1)
int(2)
int(2)
int(1)
int(2)
int(0)
int(0)
