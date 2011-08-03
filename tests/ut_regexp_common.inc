<?php
function ut_regexp_create($oo)
{
    $args = func_get_args();
    array_splice($args, 0, 1);
    if ($oo) {
        $r = new ReflectionClass('Regexp');
        return $r->newInstanceArgs($args);
    } else {
        return call_user_func_array('regexp_create', $args);
    }
}
