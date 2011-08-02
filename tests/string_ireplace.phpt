--TEST--
Test utf8_ireplace function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
# charset: UTF-8

ini_set('intl.error_level', E_WARNING);
ini_set('intl.default_locale', 'fr'); // set a default locale

define('TIMES', 30);

function assert_single($subject, $search, $replacement, $expected, $locale = '') {
    return utf8_ireplace($subject, $search, $replacement, $locale) === $expected;
}

function assert_multiple($subject, $search, $replacement, $expected, $locale = '') {
    return utf8_ireplace(str_repeat($subject, TIMES), $search, $replacement, $locale) === str_repeat($expected, TIMES);
}

var_dump(utf8_ireplace('subject', 'search'));
var_dump(utf8_ireplace('subject', 'search', 'replacement', 'locale', $count, 'FAIL'));

echo 'Check count parameter', "\n";
utf8_ireplace('oui', 'non', 'maybe', '', $c1);
var_dump($c1);
utf8_ireplace('la valeur a déjà été modifiée', 'É', 'x', '', $c2);
var_dump($c2);

echo 'Delete (replace by empty string)', "\n";
var_dump(assert_single("Vous eûtes 23 ÉlèVes", ' élèves', '', "Vous eûtes 23"));
var_dump(assert_multiple("Vous eûtes 23 ÉlèVes", ' élèves', '', "Vous eûtes 23"));

echo 'Replace by a shorter string', "\n";
var_dump(assert_single("Vous eûtes 23 ÉlèVes", 'élèves', 'cats', "Vous eûtes 23 cats"));
var_dump(assert_multiple("Vous eûtes 23 ÉlèVes", 'élèves', 'cats', "Vous eûtes 23 cats"));

echo 'ß <=> SS', "\n";
var_dump(assert_single('Petersburger Straße', 'SS', '<eszett>', 'Petersburger Stra<eszett>e'));
var_dump(assert_multiple('Petersburger Straße', 'SS', '<eszett>', 'Petersburger Stra<eszett>e'));

echo 'DZ digraph', "\n";
var_dump(assert_single('ǲwon HRÁǱA', 'Ǳ', '<DZ>', '<DZ>won HRÁ<DZ>A'));
var_dump(assert_multiple('ǲwon HRÁǱA', 'Ǳ', '<DZ>', '<DZ>won HRÁ<DZ>A'));

echo 'Turkish dotted I', "\n";
var_dump(assert_single('İyi akşamlar', 'İ', '<dotted i>', '<dotted i>y<dotted i> akşamlar', 'tr'));
var_dump(assert_multiple('İyi akşamlar', 'İ', '<dotted i>', '<dotted i>y<dotted i> akşamlar', 'tr'));

echo 'Turkish non dotted I', "\n";
var_dump(assert_single('Hayır', 'I', '<non dotted i>', 'Hay<non dotted i>r', 'tr'));
var_dump(assert_multiple('Hayır', 'I', '<non dotted i>', 'Hay<non dotted i>r', 'tr'));
?>
--EXPECTF--

Warning: utf8_ireplace() expects at least 3 parameters, 2 given in %s on line %d
NULL

Warning: utf8_ireplace() expects at most 5 parameters, 6 given in %s on line %d
NULL
Check count parameter
int(0)
int(6)
Delete (replace by empty string)
bool(true)
bool(true)
Replace by a shorter string
bool(true)
bool(true)
ß <=> SS
bool(true)
bool(true)
DZ digraph
bool(true)
bool(true)
Turkish dotted I
bool(true)
bool(true)
Turkish non dotted I
bool(true)
bool(true)
