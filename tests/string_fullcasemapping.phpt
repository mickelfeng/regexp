--TEST--
Test utf8_to[title|upper|lower] function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

define('OVERFLOW_ATTEMPT', 10000);

$functions = array('totitle', 'toupper', 'tolower');

define('INITIAL_VALUE', 'içmek');
$locales = array(
    'fr' => array('totitle' => 'Içmek', 'toupper' => 'IÇMEK', 'tolower' => 'içmek'),
    'tr' => array('totitle' => 'İçmek', 'toupper' => 'İÇMEK', 'tolower' => 'içmek'),
);

foreach ($functions as $f) {
    $f = 'utf8_' . $f;
    var_dump($f());
    var_dump($f(INITIAL_VALUE, 'en', 3));
}

echo "##### Empty string #####\n";
foreach ($functions as $f) {
    $f = 'utf8_' . $f;
    var_dump($f(''));
}

echo "##### Default locale #####\n";
foreach ($locales as $l => $a) {
    ini_set('intl.default_locale', $l);
    $word = INITIAL_VALUE;
    foreach ($functions as $f) {
        $word = call_user_func('utf8_' . $f, $word);
        echo $word === $a[$f] ? 'OK' : 'FAILED', ' (', $l, ' => ', $f, ')', "\n";
    }
}

ini_set('intl.default_locale', 'en_US');

echo "##### Explicit locale #####\n";
foreach ($locales as $l => $a) {
    $word = INITIAL_VALUE;
    foreach ($functions as $f) {
        $word = call_user_func('utf8_' . $f, $word, $l);
        echo $word === $a[$f] ? 'OK' : 'FAILED', ' (', $l, ' => ', $f, ')', "\n";
    }
}

echo "##### Full case mapping #####\n";
echo utf8_toupper('straße') === 'STRASSE' ? 'OK' : 'FAILED', ' (U+00DF)', "\n";
echo utf8_totitle('ﬁne') === 'Fine' ? 'OK' : 'FAILED', ' (U+FB01)', "\n";
echo utf8_tolower('ABCΣDEF') === 'abcσdef' ? 'OK' : 'FAILED', ' (U+03A3/U+03C3)', "\n";
echo utf8_tolower('GHIΣ') === 'ghiς' ? 'OK' : 'FAILED', ' (U+03A3/U+03C2)', "\n";

echo "##### Overflow #####\n";
echo utf8_toupper(str_repeat('ß', OVERFLOW_ATTEMPT)) === str_repeat('SS', OVERFLOW_ATTEMPT) ? 'OK' : 'FAILED', ' (*2)', "\n";
echo utf8_toupper(str_repeat('ı', OVERFLOW_ATTEMPT), 'tr') === str_repeat('I', OVERFLOW_ATTEMPT) ? 'OK' : 'FAILED', ' (/2)', "\n";
?>
--EXPECTF--

Warning: utf8_totitle() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_totitle() expects at most 2 parameters, 3 given in %s on line %d
NULL

Warning: utf8_toupper() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_toupper() expects at most 2 parameters, 3 given in %s on line %d
NULL

Warning: utf8_tolower() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: utf8_tolower() expects at most 2 parameters, 3 given in %s on line %d
NULL
##### Empty string #####
string(0) ""
string(0) ""
string(0) ""
##### Default locale #####
OK (fr => totitle)
OK (fr => toupper)
OK (fr => tolower)
OK (tr => totitle)
OK (tr => toupper)
OK (tr => tolower)
##### Explicit locale #####
OK (fr => totitle)
OK (fr => toupper)
OK (fr => tolower)
OK (tr => totitle)
OK (tr => toupper)
OK (tr => tolower)
##### Full case mapping #####
OK (U+00DF)
OK (U+FB01)
OK (U+03A3/U+03C3)
OK (U+03A3/U+03C2)
##### Overflow #####
OK (*2)
OK (/2)
