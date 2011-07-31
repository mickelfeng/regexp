--TEST--
Test utf8_shuffle function
--SKIPIF--
<?php
if (!extension_loaded('intl')) echo 'skip';
?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

$A="\xF0\x9D\x98\xBC"; # 1D63C, Lu
$B="\xF0\x9D\x98\xBD";
$C="\xF0\x9D\x98\xBE";
$D="\xF0\x9D\x98\xBF";
$E="\xF0\x9D\x99\x80";
$N0="\xF0\x9D\x9F\x8E"; # 1D7CE, Nd
$N1="\xF0\x9D\x9F\x8F";
$N2="\xF0\x9D\x9F\x90";
$N3="\xF0\x9D\x9F\x91";
$N4="\xF0\x9D\x9F\x92";
$N5="\xF0\x9D\x9F\x93";
$N6="\xF0\x9D\x9F\x94";
$N7="\xF0\x9D\x9F\x95";
$N8="\xF0\x9D\x9F\x96";
$N9="\xF0\x9D\x9F\x97";

$input = "abc{$A}{$B}{$C}{$D}{$E}{$N0}déf{$N1}{$N2}{$N3}{$N4}ç0{$N5}{$N6}{$N7}{$N8}{$N9}";

var_dump(utf8_shuffle());
var_dump(utf8_shuffle($input, 2));

if (extension_loaded('mbstring')) {
    mb_internal_encoding('UTF-8');

    function utf8_str_split($string, $length = 1) {
        $ret = array();
        $l = mb_strlen($string, 'UTF-8');

        for ($i = 0; $i < $l; $i += $length) {
            $ret[] = mb_substr($string, $i, $length, 'UTF-8');
        }

        return $ret;
    }
} else {
    function utf8_str_split($string, $length = 1) {
        return utf8_split($string, $length);
    }
}

// May be hard to rewrite an equivalent of utf8_count_chars and utf8_ord
// We will avoid to rewrite them in pure PHP in order to not introduce bugs
$res = utf8_count_chars(utf8_shuffle($input), 1);
$expected = array_fill_keys(array_map('utf8_ord', utf8_str_split($input)), 1);
ksort($res);
ksort($expected);

var_dump($res === $expected);
?>
--EXPECTF--

Warning: utf8_shuffle() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_shuffle() expects exactly 1 parameter, 2 given in %s on line %d
NULL
bool(true)
