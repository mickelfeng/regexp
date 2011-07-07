<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');

$b = new Benchmark;
$b->add(
    function ($string) {
        mb_strlen($string);
    },
    "mbstrlen (default encoding)"
);
$b->add(
    function ($string) {
        mb_strlen($string, 'UTF-8');
    },
    "mbstrlen (encoding explicited)"
)
->add(
    function ($string) {
        utf8_len($string);
    },
    "utf8"
)->registerArg($string)
->execute()
->report();