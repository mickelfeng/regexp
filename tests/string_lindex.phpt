--TEST--
Test utf8_lindex function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

var_dump(utf8_lindex("ABC{$N03B0}DEF{$N0587}GHI{$N1FB7}JKL", $C1FB7, 0, TRUE));
var_dump(utf8_lindex("ABC{$N03B0}DEF{$C0587}GHI{$N1FB7}JKL", $N0587, 0, TRUE));
?>
--EXPECTF--
int(11)
int(7)
