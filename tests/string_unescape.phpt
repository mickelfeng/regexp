--TEST--
Test utf8_unescape function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

# Invalid invocations (number of arguments)
var_dump(utf8_unescape());
var_dump(utf8_unescape('abc', FALSE, NULL));

echo "\n(non-)quiet mode:\n";
var_dump(utf8_unescape("\xA0", FALSE));
var_dump(utf8_unescape("\xF6", TRUE));

echo "\nSurrogates:\n";
var_dump(utf8_unescape('\uD835'));
var_dump(utf8_unescape('\uDE3C'));
var_dump(utf8_unescape('\uD835;\uDE3C'));
var_dump(utf8_unescape('\uDE3C\uD835'));
var_dump(utf8_unescape('\U0000D835'));
var_dump(utf8_unescape('\U0000DE3C'));

echo "\nValid escaping:\n";
var_dump(utf8_unescape('123\uD835\uDE3C456') === "123{$A}456");
var_dump(utf8_unescape('X\U0001D63DY') === "X{$B}Y");
var_dump(utf8_unescape('\xE9lève') === 'élève');

echo "\nLonger sequences:\n";
var_dump(utf8_unescape('\5726') === 'ź6');
var_dump(utf8_unescape('\xE9XXlève') === 'éXXlève');
var_dump(utf8_unescape('\u1D63D') === "\xE1\xB5\xA3D");
var_dump(utf8_unescape('\U0000110000') === "\xE1\x84\x8000");

echo "\nShorter sequences:\n";
var_dump(utf8_unescape('\x3'));
var_dump(utf8_unescape('\u000'));
var_dump(utf8_unescape('\U1234567'));

echo "\nInvalid octal sequences:\n";
#var_dump(utf8_unescape('\79') === "\a9");
var_dump(utf8_unescape('\89') === '\89');
var_dump(utf8_unescape('\119') === "\t9");
?>
--EXPECTF--

Warning: utf8_unescape() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_unescape() expects at most 2 parameters, 3 given in %s on line %d
NULL

(non-)quiet mode:

Warning: utf8_unescape(): illegal input sequence/combination of input units (U_ILLEGAL_CHAR_FOUND/12) in %s on line %d
bool(false)
bool(false)

Surrogates:
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

Valid escaping:
bool(true)
bool(true)
bool(true)

Longer sequences:
bool(true)
bool(true)
bool(true)
bool(true)

Shorter sequences:
bool(false)
bool(false)
bool(false)

Invalid octal sequences:
bool(true)
bool(true)
