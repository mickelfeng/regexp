--TEST--
Test utf8_slice_cmp function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";
$E="\xF0\x9D\x99\x80";
$F="\xF0\x9D\x99\x81";


var_dump(utf8_slice_cmp("$A$B$C$D$E", 1));
var_dump(utf8_slice_cmp("$A$B$C$D$E", 1, "$B$C", 0, -1, TRUE, NULL));

/*
echo substr_compare("abcde", "BC", 1, 2, true); // 0
echo substr_compare("abcde", "bc", 1, 3); // 1
echo substr_compare("abcde", "cd", 1, 2); // -1
echo substr_compare("abcde", "abc", 5, 1); // warning
*/

// utf8_slice_cmp(STRING, STRING_OFFSET, SUBSTRING, [ SUBSTRING_OFFSET = 0, LENGTH = utf8_len(SUBSTRING), IGNORE_CASE = FALSE ])

echo 'Standard case : no case, positive offsets', "\n";
var_dump(utf8_slice_cmp("$A$B$C$D$E", 1, "$B$C"));
var_dump(utf8_slice_cmp("$A$B$C$D$E", 1, "$B$C$A", 0, 2));
var_dump(utf8_slice_cmp("$A$B$C$D$E", 1, "$F$B$C", 1, 2));
var_dump(utf8_slice_cmp("$A$B$C$D$E", 1, "$F$B$C$A", 1, 2));

echo 'Negative string offset test', "\n";
var_dump(utf8_slice_cmp("$A$B$C$D$E", -4, "$B$C")); // [3,19] [0,7] ; A = [0,3], B = [4,7], C = [8,11], D = [12,15], E = [16,19]
var_dump(utf8_slice_cmp("$A$B$C$D$E", -4, "$B$C", 0, 2));
var_dump(utf8_slice_cmp("$A$B$C$D$E", -4, "$F$B$C", 1, 2));
var_dump(utf8_slice_cmp("$A$B$C$D$E", -4, "$F$B$C$A", 1, 2));

echo 'Negative substring offset test', "\n";
var_dump(utf8_slice_cmp("$A$B$C$D$E", -3, "$F$C$D", -2, 2));
var_dump(utf8_slice_cmp("$A$B$C$D$E", -3, "$F$C$D$A", -3, 2));

echo 'Case test', "\n";
var_dump(utf8_slice_cmp("àèïôù", 1, "ÈÏY", 0, 2, TRUE));
var_dump(utf8_slice_cmp("àèïôù", 1, "XÈÏ", 1, 2, TRUE));
var_dump(utf8_slice_cmp("àèïôù", 1, "XÈÏY", 1, 2, TRUE));

echo 'Inversed', "\n";
var_dump(utf8_slice_cmp("$B$C", 0, "$A$B$C$D$E", 1));
?>
--EXPECTF--

Warning: utf8_slice_cmp() expects at least 3 parameters, 2 given in %s on line %d
NULL

Warning: utf8_slice_cmp() expects at most 6 parameters, 7 given in %s on line %d
NULL
Standard case : no case, positive offsets
int(0)
int(0)
int(0)
int(0)
Negative string offset test
int(0)
int(0)
int(0)
int(0)
Negative substring offset test
int(0)
int(0)
Case test
int(0)
int(0)
int(0)
Inversed
int(0)
