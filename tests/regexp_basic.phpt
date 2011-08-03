--TEST--
Test Regexp::__construct/regexp_create function
--SKIPIF--
<?php if (!extension_loaded('intl') || version_compare(PHP_VERSION, '5.3.0', '<')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

$oo = FALSE;

start_test_suite:

var_dump(ut_regexp_create($oo));
var_dump(ut_regexp_create($oo, '.*', 0, 'FAIL'));

if (!$oo) {
    echo "\n";
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--

Warning: regexp_create() expects at least 1 parameter, 0 given in %s on line %d

Warning: regexp_create(): bad arguments in %s on line %d
NULL

Warning: regexp_create() expects at most 2 parameters, 3 given in %s on line %d

Warning: regexp_create(): bad arguments in %s on line %d
NULL


Warning: Regexp::__construct() expects at least 1 parameter, 0 given in %s on line %d

Warning: Regexp::__construct(): bad arguments in %s on line %d
NULL

Warning: Regexp::__construct() expects at most 2 parameters, 3 given in %s on line %d

Warning: Regexp::__construct(): bad arguments in %s on line %d
NULL
