--TEST--
Test utf8_rindex function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

$input = "$N0$N1$N2$N3$N4$N5$N6$N7$N8$N9$A$N1$N2$N3$N4$N5$N6$N7$N8$N9$B$N1$N2$N3$N4$N5$N6$N7$N8$N9$C";

var_dump(utf8_rindex($input));
var_dump(utf8_rindex($input, $N7, 0, TRUE, 'tr', NULL));
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

Warning: utf8_rindex() expects at most 5 parameters, 6 given in %s on line %d
NULL
int(27)
int(17)
int(27)
int(-1)
int(27)
int(17)
int(27)
int(-1)
