<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');

$b = new Benchmark;
$b->add(
    function ($string) {
        mb_split('//', $string);
    },
    "mb_split (default encoding)"
)
->add(
    function ($string) {
        utf8_split($string);
    },
    "utf8_split"
)
->registerArg($string)
->execute()
->report();