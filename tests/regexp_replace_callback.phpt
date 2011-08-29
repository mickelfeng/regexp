--TEST--
Test Regexp::replaceCallback function
--SKIPIF--
<?php if (!extension_loaded('intl') || version_compare(PHP_VERSION, '5.3.0', '<')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

// WARNING: take care of reference (count) if(when) use(d)
function ut_regexp_replace_callback($oo, $ro)
{
    $args = func_get_args();
    array_splice($args, 0, $oo ? 2 : 1);
    return call_user_func_array($oo ? array($ro, 'replaceCallback') : 'regexp_replace_callback', $args);
}

$oo = FALSE;

start_test_suite:

$ro = ut_regexp_create($oo, '\p{L}+');
var_dump(
    $x = ut_regexp_replace_callback(
        $oo,
        $ro,
        'ǲwon hráǳa',
        function ($matches) {
            return Normalizer::normalize($matches[0], Normalizer::FORM_KD);
            // return transliterator_create('NFD; [:Nonspacing Mark:] Remove; NFC')->transliterate($in);
        }
    )
    ===
    "Dzwon hra\xCC\x81dza" // \xCC\x81 <=> U+0301 <=> COMBINING ACUTE ACCENT
);

$map = array(
    $A => 'A',
    $B => 'B',
    $C => 'C',
    $D => 'D',
    $E => 'E',
    $F => 'F',
);
$ro = ut_regexp_create($oo, '\p{L}');
var_dump(
    $x = ut_regexp_replace_callback(
        $oo,
        $ro,
        "$A$B$C$D$E$F",
        function ($matches) use ($map) {
            return isset($map[$matches[0]]) ? $map[$matches[0]] : $matches[0];
        }
    )
    ===
    'ABCDEF'
);
$map = array_flip($map);
var_dump(
    $x = ut_regexp_replace_callback(
        $oo,
        $ro,
        'ABCDEF',
        function ($matches) use ($map) {
            return isset($map[$matches[0]]) ? $map[$matches[0]] : $matches[0];
        }
    )
    ===
    "$A$B$C$D$E$F"
);

if (!$oo) {
    echo "\n";
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

bool(true)
bool(true)
bool(true)
