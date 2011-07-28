--TEST--
Test Regexp::getFlags function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

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

function ut_regexp_get_flags($oo, $ro)
{
    $args = func_get_args();
    array_splice($args, 0, $oo ? 2 : 1);
    return call_user_func_array($oo ? array($ro, 'getFlags') : 'regexp_get_flags', $args);
}

$oo = FALSE;

start_test_suite:

$ro = ut_regexp_create($oo, '\d');
echo 'None (0):', "\n";
var_dump(ut_regexp_get_flags($oo, $ro));

var_dump(ut_regexp_get_flags($oo, $ro, FALSE));

$ro = ut_regexp_create($oo, '\p{Lt}', 'i');
printf('Insensitive (%d):' . "\n", Regexp::CASE_INSENSITIVE);
var_dump(ut_regexp_get_flags($oo, $ro) - Regexp::CASE_INSENSITIVE);

$ro = ut_regexp_create($oo,
    '^        # Start of string
    \p{Lu}{2} # 2 upper letters
    \d{3,5}   # From 3 to 5 digits
    \p{L}{3}  # 3 letters
    $         # End of string', Regexp::DOTALL | Regexp::COMMENTS);
printf('Dotall + Comments (%d):' . "\n", Regexp::DOTALL | Regexp::COMMENTS);
var_dump(ut_regexp_get_flags($oo, $ro) - (Regexp::DOTALL | Regexp::COMMENTS));

$ro = ut_regexp_create($oo, '\b[^\b]+\b', 'mw');
printf('Multiline + Unicode word (%d):' . "\n", Regexp::MULTILINE | Regexp::UWORD);
var_dump(ut_regexp_get_flags($oo, $ro) - (Regexp::MULTILINE | Regexp::UWORD));

if (!$oo) {
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--
None (0):
int(0)

Warning: regexp_get_flags() expects exactly 1 parameter, 2 given in %s on line %d

Warning: regexp_get_flags(): regexp_get_flags: bad arguments in %s on line %d
bool(false)
Insensitive (%d):
int(0)
Dotall + Comments (%d):
int(0)
Multiline + Unicode word (%d):
int(0)
None (0):
int(0)

Warning: Regexp::getFlags() expects exactly 0 parameters, 1 given in %s on line %d

Warning: Regexp::getFlags(): regexp_get_flags: bad arguments in %s on line %d
bool(false)
Insensitive (%d):
int(0)
Dotall + Comments (%d):
int(0)
Multiline + Unicode word (%d):
int(0)
