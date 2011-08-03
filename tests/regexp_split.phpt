--TEST--
Test Regexp::split function
--SKIPIF--
<?php if (!extension_loaded('intl') || version_compare(PHP_VERSION, '5.3.0', '<')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

$oo = FALSE;

start_test_suite:

$ro = ut_regexp_create($oo, "$A$B$C");
$subject = "$A$B$C$D$E$F$A$B{$C}def$A$B$C";

// Prototype
var_dump(ut_regexp_split($oo, $ro));
var_dump(ut_regexp_split($oo, $ro, 'string to cut', -1, 0, 'FAIL'));

// Limit
echo 'Limit to 1/4: ', ut_regexp_split($oo, $ro, $subject, 1) === array($subject) ? 'OK' : 'FAILED', "\n";
echo 'Limit to 2/4: ', ut_regexp_split($oo, $ro, $subject, 2) === array('', "$D$E$F$A$B{$C}def$A$B$C") ? 'OK' : 'FAILED', "\n";

// SPLIT_NO_EMPTY alone
echo 'SPLIT_NO_EMPTY: ', ut_regexp_split($oo, $ro, $subject, -1, Regexp::SPLIT_NO_EMPTY) === preg_split("/$A$B$C/u", $subject, -1, PREG_SPLIT_NO_EMPTY) ? 'OK' : 'FAILED', "\n";

// OFFSET_CAPTURE alone
echo 'OFFSET_CAPTURE: ', ut_regexp_split($oo, $ro, $subject, -1, Regexp::OFFSET_CAPTURE) === array(0 => '', 3 => "$D$E$F", 9 => 'def', 15 => '') ? 'OK' : 'FAILED', "\n";

// SPLIT_DELIM_CAPTURE alone
$ro = ut_regexp_create($oo, "($A)[$B|$H]($C)");
echo 'SPLIT_DELIM_CAPTURE: ', ut_regexp_split($oo, $ro, $subject, -1, Regexp::SPLIT_DELIM_CAPTURE) === array('', $A, $C, "$D$E$F", $A, $C, "def", $A, $C, '') ? 'OK' : 'FAILED', "\n";

// SPLIT_DELIM_CAPTURE + SPLIT_NO_EMPTY
echo 'SPLIT_DELIM_CAPTURE + SPLIT_NO_EMPTY: ', ut_regexp_split($oo, $ro, $subject, -1, Regexp::SPLIT_DELIM_CAPTURE | Regexp::SPLIT_NO_EMPTY) === array($A, $C, "$D$E$F", $A, $C, "def", $A, $C) ? 'OK' : 'FAILED', "\n";

if (!$oo) {
    echo "\n";
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--

Warning: regexp_split() expects at least 2 parameters, 1 given in %s on line %d

Warning: regexp_split(): bad arguments in %s on line %d
bool(false)

Warning: regexp_split() expects at most 4 parameters, 5 given in %s on line %d

Warning: regexp_split(): bad arguments in %s on line %d
bool(false)
Limit to 1/4: OK
Limit to 2/4: OK
SPLIT_NO_EMPTY: OK
OFFSET_CAPTURE: OK
SPLIT_DELIM_CAPTURE: OK
SPLIT_DELIM_CAPTURE + SPLIT_NO_EMPTY: OK


Warning: Regexp::split() expects at least 1 parameter, 0 given in %s on line %d

Warning: Regexp::split(): bad arguments in %s on line %d
bool(false)

Warning: Regexp::split() expects at most 3 parameters, 4 given in %s on line %d

Warning: Regexp::split(): bad arguments in %s on line %d
bool(false)
Limit to 1/4: OK
Limit to 2/4: OK
SPLIT_NO_EMPTY: OK
OFFSET_CAPTURE: OK
SPLIT_DELIM_CAPTURE: OK
SPLIT_DELIM_CAPTURE + SPLIT_NO_EMPTY: OK
