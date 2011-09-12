--TEST--
Test utf8_[lr]?trim function
--SKIPIF--
<?php if (!extension_loaded('intl'))/* || version_compare(PHP_VERSION, '5.2.5', '<'))*/ echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

require(__DIR__ . '/ut_regexp_common.inc');

$ws = array(
    "\t",     /* \t */
    "\n",     /* \n */
    chr(0xB), /* \v ("\v" implies php >= 5.2.5) */
    chr(0xC), /* \f ("\f" implies php >= 5.2.5) */
    "\r",     /* \r */
    ' ',
    "\xC2\x85",     /* U+0085 */
    "\xC2\xA0",     /* U+00A0 ! */
    "\xE1\x9A\x80", /* U+1680 */
    "\xE1\xA0\x8E", /* U+180E */
    "\xE2\x80\x80", /* U+2000 ! */
    "\xE2\x80\x81", /* U+2001 ! */
    "\xE2\x80\x82", /* U+2002 ! */
    "\xE2\x80\x83", /* U+2003 ! */
    "\xE2\x80\x84", /* U+2004 ! */
    "\xE2\x80\x85", /* U+2005 ! */
    "\xE2\x80\x86", /* U+2006 ! */
    "\xE2\x80\x87", /* U+2007 ! */
    "\xE2\x80\x88", /* U+2008 ! */
    "\xE2\x80\x89", /* U+2009 ! */
    "\xE2\x80\x8A", /* U+200A ! */
    "\xE2\x80\xA8", /* U+2028 ! */
    "\xE2\x80\xA9", /* U+2029 ! */
    "\xE2\x80\xAF", /* U+202F ! */
    "\xE2\x81\x9F", /* U+205F ! */
    "\xE3\x80\x80", /* U+3000 ! */
);

shuffle($ws);

$ws = implode($ws);

$base = "$A$B$C";

$string = $ws . $base . $ws;
$expected_left = $base . $ws;
$expected_right = $ws . $base;
$expected_both = $base;

var_dump(utf8_trim());
var_dump(utf8_trim($string, $ws, NULL));

echo "Trimming white spaces:\n";
echo utf8_trim($string) === $expected_both ? 'OK' : 'FAILED', ' (both)', "\n";
echo utf8_rtrim($string) === $expected_right ? 'OK' : 'FAILED', ' (right)', "\n";
echo utf8_ltrim($string) === $expected_left ? 'OK' : 'FAILED', ' (left)', "\n";

$cu = array(
    1 => array('a', 'b', 'c', 'd'),
    array('é', 'è', 'ô', 'ß'),
    array("\xE2\xA1\x80", "\xE2\xA1\x81", "\xE2\xA1\x82", "\xE2\xA1\x83"),
    array($A, $B, $C, $D),
);

echo "Trimming based on second argument:\n";
foreach ($cu as $l => $v) {

    shuffle($v);
    $v = implode($v);

    $base = "RÉFÉRENCE";
    $string = $v . $base . $v;
    $expected_both = $base;
    $expected_left = $base . $v;
    $expected_right = $v . $base;

    echo utf8_trim($string, $v) === $expected_both ? 'OK' : 'FAILED', " ($l CU, both)\n";
    echo utf8_rtrim($string, $v) === $expected_right ? 'OK' : 'FAILED', " ($l CU, right)\n";
    echo utf8_ltrim($string, $v) === $expected_left ? 'OK' : 'FAILED', " ($l CU, left)\n";
}
?>
--EXPECTF--

Warning: utf8_trim() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_trim() expects at most 2 parameters, 3 given in %s on line %d
NULL
Trimming white spaces:
OK (both)
OK (right)
OK (left)
Trimming based on second argument:
OK (1 CU, both)
OK (1 CU, right)
OK (1 CU, left)
OK (2 CU, both)
OK (2 CU, right)
OK (2 CU, left)
OK (3 CU, both)
OK (3 CU, right)
OK (3 CU, left)
OK (4 CU, both)
OK (4 CU, right)
OK (4 CU, left)
