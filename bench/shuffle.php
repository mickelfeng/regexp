<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');

$b = new Benchmark;
$b->add(
    function ($string) {
        $cp = mb_split('//', $string);
        shuffle($cp);
        implode($cp);
    },
    "mb_split/shuffle/implode (default encoding)"
)
->add(
    function ($string) {
        utf8_shuffle($string);
    },
    "utf8_shuffle"
)
->add(
    function ($string) {
        $cp = utf8_split($string);
        shuffle($cp);
        implode($cp);
    },
    "utf8_split/shuffle/implode"
)
->registerArg($string)
->execute()
->report();
