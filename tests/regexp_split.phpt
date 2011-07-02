--TEST--
Test Regexp::split function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

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
        $r->newInstanceArgs($args);
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

var_dump(ut_regexp_create($oo));

#$ro = ut_regexp_create($oo, PATTERN/*, FLAGS*/);
#/*RESULT = */ut_regexp_split($oo, $ro, SUBJECT, OPTIONS);

if (!$oo) {
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--

TODO
