--TEST--
Test Regexp::match prototype's function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

ini_set('intl.error_level', 0); // will be sufficient

$ro = new Regexp('.');

echo 'Procedural way', "\n";
var_dump(regexp_match($ro)); // KO
var_dump(regexp_match($ro, "$A$B$C", $match, Regexp::OFFSET_CAPTURE, 1)); // OK
var_dump(regexp_match($ro, "$A$B$C", $match, Regexp::OFFSET_CAPTURE, 1, FALSE)); // KO

echo "\n";
echo 'OO way', "\n";
var_dump($ro->match()); // KO
var_dump($ro->match("$A$B$C", $match, Regexp::OFFSET_CAPTURE, 1)); // OK
var_dump($ro->match("$A$B$C", $match, Regexp::OFFSET_CAPTURE, 1, FALSE)); // KO
?>
--EXPECTF--
Procedural way

Warning: regexp_match() expects at least 2 parameters, 1 given in %s on line %d
bool(false)
bool(true)

Warning: regexp_match() expects at most 5 parameters, 6 given in %s on line %d
bool(false)

OO way

Warning: Regexp::match() expects at least 1 parameter, 0 given in %s on line %d
bool(false)
bool(true)

Warning: Regexp::match() expects at most 4 parameters, 5 given in %s on line %d
bool(false)
