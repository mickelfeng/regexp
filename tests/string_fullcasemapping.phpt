--TEST--
Test utf8_to[title|upper|lower] function
--SKIPIF--
<?php if (!extension_loaded('intl')) echo 'skip'; ?>
--FILE--
<?php
ini_set('intl.error_level', E_WARNING);

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

echo "##### Explicit locale #####\n";
foreach ($locales as $l => $a) {
    $word = INITIAL_VALUE;
    foreach ($functions as $f) {
        $word = call_user_func('utf8_' . $f, $word, $l);
        echo $word === $a[$f] ? 'OK' : 'FAILED', ' (', $l, ' => ', $f, ')', "\n";
    }
}
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
