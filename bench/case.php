<?php
require(__DIR__ . '/Benchmark.php');
require(__DIR__ . '/header.php');


$b = new Benchmark;
$b->add(
    function ($string) {
        mb_strtoupper($string);
    },
    "mb_strtoupper (default encoding)"
)
->add(
    function ($string) {
        mb_convert_case($string, MB_CASE_UPPER);
    },
    "mb_convertcase (default encoding)"
)
->add(
    function ($string) {
        utf8_toupper($string);
    },
    "utf8_toupper (locale = '')"
)
->registerArg($string)
->execute()
->report();
