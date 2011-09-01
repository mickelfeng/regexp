--TEST--
Test utf8_rindex function
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

$input = "$N0$N1$N2$N3$N4$N5$N6$N7$N8$N9$A$N1$N2$N3$N4$N5$N6$N7$N8$N9$B$N1$N2$N3$N4$N5$N6$N7$N8$N9$C";

var_dump(utf8_rindex($input));
var_dump(utf8_rindex($input, $N7, 0, TRUE, NULL));
# needle as string
var_dump(utf8_rindex($input, $N7)); // 27
var_dump(utf8_rindex($input, $N7, -5)); // 17
var_dump(utf8_rindex($input, $N7, 20)); // 27
var_dump(utf8_rindex($input, $N7, 28)); // not found : -1
# needle as long/cp
var_dump(utf8_rindex($input, 0x1D7D5)); // 27
var_dump(utf8_rindex($input, 0x1D7D5, -5)); // 17
var_dump(utf8_rindex($input, 0x1D7D5, 20)); // 27
var_dump(utf8_rindex($input, 0x1D7D5, 28)); // not found : -1
?>
--EXPECTF--
Warning: utf8_rindex() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: utf8_rindex() expects at most 4 parameters, 5 given in %s on line %d
NULL
int(27)
int(17)
int(27)
int(-1)
int(27)
int(17)
int(27)
int(-1)
