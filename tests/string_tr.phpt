--TEST--
Test utf8_tr function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";
$E="\xF0\x9D\x99\x80";
$N0="\xF0\x9D\x9F\x8E"; # 1D7CE, Nd
$N1="\xF0\x9D\x9F\x8F";
$N2="\xF0\x9D\x9F\x90";
$N3="\xF0\x9D\x9F\x91";
$N4="\xF0\x9D\x9F\x92";
$N5="\xF0\x9D\x9F\x93";
$N6="\xF0\x9D\x9F\x94";
$N7="\xF0\x9D\x9F\x95";
$N8="\xF0\x9D\x9F\x96";
$N9="\xF0\x9D\x9F\x97";

var_dump(utf8_tr("$A$B$C", $A));
var_dump(utf8_tr("$A$B$C", $A, $B, FALSE));

// overflow attempt (size grows to 4 times)
$count = 1000;
echo 'Overflow attempt: ', utf8_tr(str_repeat('a', $count), 'a', $A) === str_repeat($A, $count) ? 'OK' : 'FAILED', "\n";

echo '1 => 1: ', utf8_tr("a{$A}b{$B}c{$C}d{$D}e{$E}", "abcde", "12345") === "1{$A}2{$B}3{$C}4{$D}5{$E}" ? 'OK' : 'FAILED', "\n";
echo '1 => 2: ', utf8_tr("a{$A}b{$B}c{$C}d{$D}e{$E}", "abcde", "$N1$N2$N3$N4$N5") === "$N1$A$N2$B$N3$C$N4$D$N5$E" ? 'OK' : 'FAILED', "\n";
echo '2 => 1: ', utf8_tr("a{$A}b{$B}c{$C}d{$D}e{$E}", "$A$B$C$D$E", "12345") === "a1b2c3d4e5" ? 'OK' : 'FAILED', "\n";
echo '2 => 2: ', utf8_tr("a{$A}b{$B}c{$C}d{$D}e{$E}", "$A$B$C$D$E", "$N1$N2$N3$N4$N5") === "a{$N1}b{$N2}c{$N3}d{$N4}e{$N5}" ? 'OK' : 'FAILED', "\n";
?>
--EXPECTF--

Warning: utf8_tr() expects exactly 3 parameters, 2 given in %s on line %d
NULL

Warning: utf8_tr() expects exactly 3 parameters, 4 given in %s on line %d
NULL
Overflow attempt: OK
1 => 1: OK
1 => 2: OK
2 => 1: OK
2 => 2: OK
