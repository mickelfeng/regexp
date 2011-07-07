<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');


$b = new Benchmark;
$b->add(
    function ($string) {
        $ret = array();
        $l = mb_strlen($string);

        for ($i = 0; $i < $l; $i++) {
            $ret[] = mb_substr($string, $i, 1);
        }
    },
    "mb_strlen/mb_substr (length = 1, default encoding)"
)
->add(
    function ($string) {
        utf8_split($string);
    },
    "utf8_split (length = 1)"
)
->registerArg($string)
->execute()
->report();


$length = rand(2, utf8_len($string));
$b = new Benchmark;
$b->add(
    function ($string, $length) {
        $ret = array();
        $l = mb_strlen($string);

        for ($i = 0; $i < $l; $i += $length) {
            $ret[] = mb_substr($string, $i, $length);
        }
    },
    "mb_strlen/mb_substr (random length, default encoding)"
)
->add(
    function ($string, $length) {
        utf8_split($string, $length);
    },
    "utf8_split (random length)"
)
->registerArg($string, $length)
->execute()
->report();