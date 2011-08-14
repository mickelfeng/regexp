<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');

preg_match_all('~<prénom>.*?</prénom>~iu', '');
$ro = new Regexp('<prénom>.*?</prénom>', 'i');

$contents = file_get_contents(__DIR__ . '/test.xml');

$b = new Benchmark;
$b->add(
    function ($string) {
        preg_match_all('~<prénom>.*?</prénom>~iu', $string);
    },
    "PCRE",
    array(
        $contents,
    )
)
->add(
    function ($ro, $string) {
        $ro->matchAll($string);
    },
    "INTL extended",
    array(
        $ro,
        $contents,
    )
)
->execute()
->report();

/*
With UText :
------------------------------------------------------------
PCRE : 3.385778
------------------------------------------------------------
INTL extended : 7.703852 (~ 2)
============================================================
Best  : PCRE
Worst : INTL extended

With conversions UTF-8 <=> UTF-16
------------------------------------------------------------
PCRE : 3.384665
------------------------------------------------------------
INTL extended : 5.084954 (~ 1)
============================================================
Best  : PCRE
Worst : INTL extended
*/