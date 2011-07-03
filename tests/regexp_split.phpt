--TEST--
Test Regexp::split function
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

function ut_regexp_create($oo)
{
    $args = func_get_args();
    array_splice($args, 0, 1);
    if ($oo) {
        $r = new ReflectionClass('Regexp');
        return $r->newInstanceArgs($args);
    } else {
        return call_user_func_array('regexp_create', $args);
    }
}

function ut_regexp_split($oo, $ro)
{
    $args = func_get_args();
    array_splice($args, 0, $oo ? 2 : 1);
    return call_user_func_array($oo ? array($ro, 'split') : 'regexp_split', $args);
}

$oo = FALSE;

start_test_suite:

ini_set('intl.error_level', 0); // double errors, so disable this temporary
var_dump(ut_regexp_create($oo));

ini_set('intl.error_level', E_WARNING);

$ro = ut_regexp_create($oo, "$A$B$C");
$subject = "$A$B$C$D$E$F$A$B{$C}def$A$B$C";

# Limit
echo 'Limit to 1/4: ', ut_regexp_split($oo, $ro, $subject, 1) === array($subject) ? 'OK' : 'FAILED', "\n";
echo 'Limit to 2/4: ', ut_regexp_split($oo, $ro, $subject, 2) === array('', "$D$E$F$A$B{$C}def$A$B$C") ? 'OK' : 'FAILED', "\n";

# SPLIT_NO_EMPTY alone
echo 'SPLIT_NO_EMPTY: ', ut_regexp_split($oo, $ro, $subject, -1, Regexp::SPLIT_NO_EMPTY) === preg_split("/$A$B$C/u", $subject, -1, PREG_SPLIT_NO_EMPTY) ? 'OK' : 'FAILED', "\n";

# OFFSET_CAPTURE alone
echo 'OFFSET_CAPTURE: ', ut_regexp_split($oo, $ro, $subject, -1, Regexp::OFFSET_CAPTURE) === array(0 => '', 3 => "$D$E$F", 9 => 'def', 15 => '') ? 'OK' : 'FAILED', "\n";

# SPLIT_DELIM_CAPTURE alone
# TODO, when implemented

if (!$oo) {
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--
Warning: regexp_create() expects at least 1 parameter, 0 given in %s on line %d
NULL
Limit to 1/4: OK
Limit to 2/4: OK
SPLIT_NO_EMPTY: OK
OFFSET_CAPTURE: OK

Warning: Regexp::__construct() expects at least 1 parameter, 0 given in %s on line %d
NULL
Limit to 1/4: OK
Limit to 2/4: OK
SPLIT_NO_EMPTY: OK
OFFSET_CAPTURE: OK
