<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');

$count = rand(0, count($myvars));

function array_rand_extract($array, $count) {
    shuffle($array);
    return array_slice($array, 0, $count);
}

$fromA = array_rand_extract($myvars, $count);
$toA = array_rand_extract($myvars, $count);

$from = implode($fromA);
$to = implode($toA);

$map = array_combine($fromA, $toA);


$b = new Benchmark;
$b->add(
    function ($string, $map) {
        strtr($string, $map);
    },
    "strtr (from/to as array)",
    array(
        $string,
        $map,
    )
)
->add(
    function ($string, $from, $to) {
        utf8_tr($string, $from, $to);
    },
    "utf8_tr",
    array(
        $string,
        $from,
        $to,
    )
)
->execute()
->report();
