--TEST--
Test Regexp::match function
--SKIPIF--
<?php if (!extension_loaded('intl') || version_compare(PHP_VERSION, '5.3.0', '<')) echo 'skip'; ?>
--FILE--
<?php
require(__DIR__ . '/ut_regexp_common.inc');

function ut_regexp_match($oo, $ro, $subject, &$matches, $flags = 0, $start_offset = 0)
{
    if ($oo) {
        return call_user_func_array(array($ro, 'match'), array($subject, &$matches, $flags, $start_offset));
    } else {
        return call_user_func_array('regexp_match', array($ro, $subject, &$matches, $flags, $start_offset));
    }
}

$oo = FALSE;

start_test_suite:

echo 'Match a single line in a multiline text (. doesn\'t include \n by default)', "\n";
$ro = ut_regexp_create($oo, 'M(.*)');
$string = "My\nName\nIs\nStrange";
ut_regexp_match($oo, $ro, $string, $matches);
var_dump($matches);

echo "\n";
echo 'If the same variable was used, is it updated? (expected: yes)', "\n";
$string = "-1";
$ro = ut_regexp_create($oo, '[\-\+]?[0-9\.]*');
ut_regexp_match($oo, $ro, $string, $string);
var_dump($string);

$string = "$H$e$l$l$o, $w$o$r$l$d. [*], $t$h$i$s $i$s \ $a $s$t$r$i$n$g";

echo "\n";
echo 'Finds "Hello, "', "\n";
$ro = ut_regexp_create($oo, "^[$h$H]$e$l$l$o,\s");
var_dump(ut_regexp_match($oo, $ro, $string, $match1));
var_dump($match1);

echo "\n";
echo 'Tries to find "lo, world" at start of string', "\n";
$ro = ut_regexp_create($oo, "$l^$o,\s\w{5}");
var_dump(ut_regexp_match($oo, $ro, $string, $match2, Regexp::OFFSET_CAPTURE));
var_dump($match2);

echo "\n";
echo 'Finds "[*], this is \ a string"', "\n";
$ro = ut_regexp_create($oo, '\[\*\],\s(.*)');
var_dump(ut_regexp_match($oo, $ro, $string, $match3));
var_dump($match3);

echo "\n";
echo 'Finds "this is \ a string" (with non-capturing parentheses)', "\n";
$ro = ut_regexp_create($oo, '\w{4}\s\w{2}\s\\\(?:\s.*)');
var_dump(ut_regexp_match($oo, $ro, $string, $match4, Regexp::OFFSET_CAPTURE, 14));
var_dump($match4);

echo "\n";
echo 'Tries to find "hello world" (should be Hello, world)', "\n";
$ro = ut_regexp_create($oo, "$h$e$l$l$o $w$o$r$l$d");
var_dump(ut_regexp_match($oo, $ro, $string, $match5));
var_dump($match5);

echo "\n";
echo 'Match a single line in a multiline text + offsets test', "\n";
$ro = ut_regexp_create($oo, "$B.+");
$string = "$B$A\n$B$B\n$B$C\n$B$D\n$B$E\n$B$F";
var_dump(ut_regexp_match($oo, $ro, $string, $match6, Regexp::OFFSET_CAPTURE, 4));
var_dump($match6);

if (!$oo) {
    echo "\n";
    $oo = TRUE;
    goto start_test_suite;
}
?>
--EXPECTREGEX--
Match a single line in a multiline text \(\. doesn't include \\n by default\)
array\(2\) \{
  \[0\]=>
  string\(2\) \"My\"
  \[1\]=>
  string\(1\) \"y\"
\}

If the same variable was used, is it updated\? \(expected: yes\)
array\(1\) \{
  \[0\]=>
  string\(2\) \"-1\"
\}

Finds \"Hello, \"
bool\(true\)
array\(1\) \{
  \[0\]=>
  string\(22\) \"\xF0\x9D\x99\x83\xF0\x9D\x98\xA6\xF0\x9D\x98\xAD\xF0\x9D\x98\xAD\xF0\x9D\x98\xB0\, "
}

Tries to find \"lo, world\" at start of string
bool\(false\)
array\(0\) \{
\}

Finds \"\[\*\], this is \\ a string\"
bool\(true\)
array\(2\) \{
  \[0\]=>
  string\(62\) \"\[\*\], \xF0\x9D\x98\xB5\xF0\x9D\x98\xA9\xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \\ \xF0\x9D\x98\xA2 \xF0\x9D\x98\xB4\xF0\x9D\x98\xB5\xF0\x9D\x98\xB3\xF0\x9D\x98\xAA\xF0\x9D\x98\xAF\xF0\x9D\x98\xA8\"
  \[1\]=>
  string\(57\) \"\xF0\x9D\x98\xB5\xF0\x9D\x98\xA9\xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \\ \xF0\x9D\x98\xA2 \xF0\x9D\x98\xB4\xF0\x9D\x98\xB5\xF0\x9D\x98\xB3\xF0\x9D\x98\xAA\xF0\x9D\x98\xAF\xF0\x9D\x98\xA8\"
\}

Finds \"this is \\ a string\" \(with non-capturing parentheses\)
bool\(true\)
array\(1\) \{
  \[19\]=>
  string\(57\) \"\xF0\x9D\x98\xB5\xF0\x9D\x98\xA9\xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \\ \xF0\x9D\x98\xA2 \xF0\x9D\x98\xB4\xF0\x9D\x98\xB5\xF0\x9D\x98\xB3\xF0\x9D\x98\xAA\xF0\x9D\x98\xAF\xF0\x9D\x98\xA8\"
}

Tries to find \"hello world\" \(should be Hello, world\)
bool\(false\)
array\(0\) \{
\}

Match a single line in a multiline text \+ offsets test
bool\(true\)
array\(1\) \{
  \[6\]=>
  string\(8\) \"\xF0\x9D\x98\xBD\xF0\x9D\x98\xBE\"
\}

Match a single line in a multiline text \(\. doesn't include \\n by default\)
array\(2\) \{
  \[0\]=>
  string\(2\) \"My\"
  \[1\]=>
  string\(1\) \"y\"
\}

If the same variable was used, is it updated\? \(expected: yes\)
array\(1\) \{
  \[0\]=>
  string\(2\) \"-1\"
\}

Finds \"Hello, \"
bool\(true\)
array\(1\) \{
  \[0\]=>
  string\(22\) \"\xF0\x9D\x99\x83\xF0\x9D\x98\xA6\xF0\x9D\x98\xAD\xF0\x9D\x98\xAD\xF0\x9D\x98\xB0\, "
}

Tries to find \"lo, world\" at start of string
bool\(false\)
array\(0\) \{
\}

Finds \"\[\*\], this is \\ a string\"
bool\(true\)
array\(2\) \{
  \[0\]=>
  string\(62\) \"\[\*\], \xF0\x9D\x98\xB5\xF0\x9D\x98\xA9\xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \\ \xF0\x9D\x98\xA2 \xF0\x9D\x98\xB4\xF0\x9D\x98\xB5\xF0\x9D\x98\xB3\xF0\x9D\x98\xAA\xF0\x9D\x98\xAF\xF0\x9D\x98\xA8\"
  \[1\]=>
  string\(57\) \"\xF0\x9D\x98\xB5\xF0\x9D\x98\xA9\xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \\ \xF0\x9D\x98\xA2 \xF0\x9D\x98\xB4\xF0\x9D\x98\xB5\xF0\x9D\x98\xB3\xF0\x9D\x98\xAA\xF0\x9D\x98\xAF\xF0\x9D\x98\xA8\"
\}

Finds \"this is \\ a string\" \(with non-capturing parentheses\)
bool\(true\)
array\(1\) \{
  \[19\]=>
  string\(57\) \"\xF0\x9D\x98\xB5\xF0\x9D\x98\xA9\xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \xF0\x9D\x98\xAA\xF0\x9D\x98\xB4 \\ \xF0\x9D\x98\xA2 \xF0\x9D\x98\xB4\xF0\x9D\x98\xB5\xF0\x9D\x98\xB3\xF0\x9D\x98\xAA\xF0\x9D\x98\xAF\xF0\x9D\x98\xA8\"
}

Tries to find \"hello world\" \(should be Hello, world\)
bool\(false\)
array\(0\) \{
\}

Match a single line in a multiline text \+ offsets test
bool\(true\)
array\(1\) \{
  \[6\]=>
  string\(8\) \"\xF0\x9D\x98\xBD\xF0\x9D\x98\xBE\"
\}
