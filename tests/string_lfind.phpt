--TEST--
Test utf8_lfind function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

$email  = 'AbcCdEfGh';
var_dump(utf8_lfind($email, 'c', 0, FALSE, TRUE));
var_dump(utf8_lfind($email, 'c', 0, TRUE, TRUE));

$email  = 'AbCdeEfGh';
var_dump(utf8_lfind($email, 'E', 0, FALSE, TRUE));
var_dump(utf8_lfind($email, 'E', 0, TRUE, TRUE));

$email  = 'wazAbCdeEfGh';
var_dump(utf8_lfind($email, 97, 0, FALSE, TRUE));
var_dump(utf8_lfind($email, 97, 0, TRUE, TRUE));
?>
--EXPECTF--
string(7) "cCdEfGh"
string(2) "Ab"
string(5) "eEfGh"
string(4) "AbCd"
string(11) "azAbCdeEfGh"
string(1) "w"
