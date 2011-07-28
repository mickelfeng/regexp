--TEST--
Test Regexp::match function
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

$H = "\xF0\x9D\x99\x83"; # 1D643

$a = "\xF0\x9D\x98\xA2"; # 1D622
$d = "\xF0\x9D\x98\xA5"; # 1D625
$e = "\xF0\x9D\x98\xA6"; # 1D626
$g = "\xF0\x9D\x98\xA8"; # 1D628
$h = "\xF0\x9D\x98\xA9"; # 1D629
$i = "\xF0\x9D\x98\xAA"; # 1D62A
$l = "\xF0\x9D\x98\xAD"; # 1D62D
$n = "\xF0\x9D\x98\xAF"; # 1D62F
$o = "\xF0\x9D\x98\xB0"; # 1D630
$r = "\xF0\x9D\x98\xB3"; # 1D633
$s = "\xF0\x9D\x98\xB4"; # 1D634
$t = "\xF0\x9D\x98\xB5"; # 1D635
$w = "\xF0\x9D\x98\xB8"; # 1D638

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

function ut_regexp_match($oo, $ro, $subject, &$matches, $flags = 0)
{
    if ($oo) {
        return call_user_func_array(array($ro, 'match'), array($subject, &$matches, $flags));
    } else {
        return call_user_func_array('regexp_match', array($ro, $subject, &$matches, $flags));
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
